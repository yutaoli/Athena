#include "CMysql.h"
#include <stdio.h>
#include <iostream>
#include <sstream>

mysql_slopover::mysql_slopover(const string& s): logic_error(s) {}
mysql_execfail::mysql_execfail(const string& s): runtime_error(s) {}


////////////////////////////////////////////////
	MySqlRowData::MySqlRowData(const vector<string>& data, map<string, int>& s2n)
: _data(&data), _s2n(&s2n)
{}

	MySqlRowData::MySqlRowData(const MySqlRowData& right)
: _data(right._data), _s2n(right._s2n)
{}

void MySqlRowData::operator=(const MySqlRowData& right)
{
	_data = right._data;
	_s2n = right._s2n;
}

const string& MySqlRowData::operator [](const string& s) const throw(mysql_slopover)
{
	if ((*_s2n).find(s) == (*_s2n).end())
		throw mysql_slopover(s + " slopover in MySqlRowData");
	return (*_data)[(*_s2n)[s]];
}

////////////////////////////////////////////////
MySqlBasicData::MySqlBasicData()
{
	_affected_rows = 0;
	_nRefCount = 0;
	//  cerr << "new " << this << endl;
}

void MySqlBasicData::RefAdd()
{_nRefCount++;}

void MySqlBasicData::RefSub()
{if (--_nRefCount == 0) delete this;}

void MySqlBasicData::Fields(const vector<string>& v) throw(mysql_slopover)
{
	_col.clear(); _s2n.clear();
	for (size_t i = 0; i < v.size(); i++) {
		if (_s2n.find(v[i]) != _s2n.end()) {
			throw mysql_slopover(string("MySqlBasicData::Fields ") + v[i] + " duplicate");
		}
		_col.push_back(v[i]);
		_s2n[v[i]] = i;
	}
}

void MySqlBasicData::push_back(vector<string>& v) throw(mysql_slopover)
{
	if (v.size() == 0 || v.size() != _col.size()) {
		throw mysql_slopover("MySqlBasicData::push_back: num is not match");
	}
	_data.push_back(v);
}

// do it after push_back over
void MySqlBasicData::genrows()
{
	_rows.clear();
	for (size_t i = 0; i < _data.size(); i++)
		_rows.push_back(MySqlRowData(_data[i], _s2n));
}

////////////////////////////////////////////////
	MySqlData::MySqlData(const MySqlData& right)
: _data(right._data)
{
	_data->RefAdd();
}

void MySqlData::operator = (const MySqlData& right)
{
	if (_data) _data->RefSub();
	_data = right._data;
	_data->RefAdd();
}

	MySqlData::MySqlData(MySqlBasicData* data)
: _data(data)
{
	_data->RefAdd();
}

MySqlData::~MySqlData()
{
	_data->RefSub();
}

const MySqlRowData& MySqlData::operator [](const size_t row) const  throw(mysql_slopover)
{
	if (row >= (_data->_rows).size()) {
		char sTmp[16];
		snprintf(sTmp, sizeof(sTmp), "%u", row);
		throw mysql_slopover(string("MySqlRowData::[") + sTmp + "] slopover");
	}
	return (_data->_rows)[row];
}

//
size_t MySqlData::affected_rows() const {return _data->_affected_rows;}
bool MySqlData::has_autoincrement_id() const {return _data->has_autoincrement_id();}
int MySqlData::get_insert_autoincrement_id() const {return _data->_autoincrement_id;}
size_t MySqlData::num_rows() const {return _data->_data.size();}
size_t MySqlData::num_fields() const {return _data->_col.size();}
string MySqlData::org_name() const {return _data->_org_name;}
const vector<string>& MySqlData::Fields() const {return _data->_col;}

////////////////////////////////////////////////
CMySql::CMySql()
{
	_Mysql = NULL;
	_bIsConn = false;
	_port = 0;
}

CMySql::~CMySql()
{
	Close();
	delete _Mysql;
	_Mysql = NULL;
}

void CMySql::Init(const string& host, const string& user, const string& pass, unsigned short port, const string& charSet, int connect_timeout, int read_timeout, int write_timeout) throw(mysql_execfail)
{
	Close();
	delete _Mysql;
	_Mysql = NULL;

	_host = host;
	_user = user;
	_pass = pass;
	_port = port;
	_charSet = charSet;

    _connect_timeout_s = connect_timeout;
    _read_timeout_s = read_timeout;
    _write_timeout_s = write_timeout;

	_Mysql = new MYSQL;
	Connect();
}

void CMySql::use(const string& db)
{
	_dbname = db;
}

void CMySql::setCharSet(const string& cs)throw(mysql_execfail)
{
	if (!_bIsConn) {
		_charSet = cs;
		return;
	}

	Close();
	_charSet = cs;
	Connect();

	if (_dbname.empty()) return;

	if (mysql_select_db(_Mysql, _dbname.c_str())) {
		int ret_errno = mysql_errno(_Mysql);
		Close();
		if (ret_errno == 2013 || ret_errno == 2006) { // CR_SERVER_LOST������һ��
			Connect();
		}

		if (mysql_select_db(_Mysql, _dbname.c_str())){
            _errno = mysql_errno(_Mysql);
			throw mysql_execfail(string("CMySql::Select: mysql_select_db ") + _dbname + ":" + mysql_error(_Mysql));
	    }
    }
}

MySqlData CMySql::query(const string& sql) throw(mysql_execfail)
{
	Select();

	if (mysql_real_query(_Mysql, sql.c_str(), sql.length())) {
		string err(mysql_error(_Mysql) + string(":") + sql);
		int ret_errno = _errno = mysql_errno(_Mysql);
		Close();
		if (ret_errno == 2013 || ret_errno == 2006) { // CR_SERVER_LOST������һ��
			Connect();
			if (mysql_select_db(_Mysql, _dbname.c_str())){
				_errno = mysql_errno(_Mysql);
				throw mysql_execfail(string("CMySql::query: mysql_select_db ") + _dbname + ":" + mysql_error(_Mysql));
			}
			if (mysql_real_query(_Mysql, sql.c_str(), sql.length())){
				_errno = mysql_errno(_Mysql);
				throw mysql_execfail(string("CMySql::query: ") + mysql_error(_Mysql) + "|" + err);
			}
		} else {
			throw mysql_execfail(string("CMySql::query: ") + err);
		}
	}

	MySqlBasicData* data = new MySqlBasicData();
	// store
	if (mysql_field_count(_Mysql) == 0) { //
		int inum = mysql_affected_rows(_Mysql);
		if (inum < 0) {
			_errno = mysql_errno(_Mysql);
			string err(sql);
			delete data;
			throw mysql_execfail(string("CMySql::query: ") + mysql_error(_Mysql) + "|" + err);
		}
		data->affected_rows(inum);

		int used_id = mysql_insert_id(_Mysql);//Ϊ0��ʾ����auto_increase value
		data->set_insert_autoincrement_id(used_id);
		return MySqlData(data);
	}
	MYSQL_RES*   pstMySqlRes = mysql_store_result(_Mysql);
	if (pstMySqlRes == NULL)  {
		_errno = mysql_errno(_Mysql);
		string err(sql);
		delete data;
		throw mysql_execfail(string("CMySql::query: mysql_store_result is null: ") + mysql_error(_Mysql) + "|" + err);
	}

	// fields
	MYSQL_FIELD* field; unsigned i = 0;
	vector<string> vfield;
	while ((field = mysql_fetch_field(pstMySqlRes))) {
		//      if(i==0) {data->org_name(field->org_name); i++;}
		// 2007-01-24,mysql�߰汾�ָֻ���table�ֶΣ����ڸù��ܵĲ��ȶ��ԣ�����֧�ִ�api
		if (i == 0) {data->org_name("not support"); i++;}
		/*
		// 2004-12-20,mysql 4.0���²�֧��org_name
#if MYSQL_VERSION_ID > 40027
if(i==0) {data->org_name(field->org_name); i++;}
#else
if(i==0) {data->org_name(field->table); i++;}
#endif
		 */
		vfield.push_back(field->name);
	}
	try {
		data->Fields(vfield);
	} catch (mysql_slopover& e) {
		delete data;
		mysql_free_result(pstMySqlRes);
		throw mysql_execfail(string("CMySql::query: catch mysql_slopover:") + e.what());
	}

	// values
	data->clear();
	MYSQL_ROW row;  vector<string> vrow;
	try {
		while ((row = mysql_fetch_row(pstMySqlRes))) {
			vrow.clear();
			unsigned long* lengths = mysql_fetch_lengths(pstMySqlRes);
			for (i = 0; i < vfield.size(); i++) {
				string s;
				if (row[i]) {
					s = string(row[i], lengths[i]);
				} else {
					s = "";
				}
				vrow.push_back(s);
			}
			data->push_back(vrow);
		}
	} catch (mysql_slopover& e) {
		delete data;
		mysql_free_result(pstMySqlRes);
		throw mysql_execfail(string("CMySql::query: catch mysql_slopover: ") + e.what());
	}
	data->genrows();
	mysql_free_result(pstMySqlRes);

	return MySqlData(data);
}

////////protected
void CMySql::Connect()
{
	mysql_init(_Mysql);

	//�������Ӻ�, �Զ����������ַ������
	if (!_charSet.empty())  {
		if (mysql_options(_Mysql, MYSQL_SET_CHARSET_NAME, _charSet.c_str())) {
			_errno = mysql_errno(_Mysql);
			throw mysql_execfail(string("CMySql::Connect: mysql_options MYSQL_SET_CHARSET_NAME ") + _charSet + ":" + mysql_error(_Mysql));
		} 

	}
	//�������ӳ�ʱʱ��
	if (mysql_options(_Mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&_connect_timeout_s)) {
		_errno = mysql_errno(_Mysql);
        std::ostringstream oss;
        oss << _connect_timeout_s;
		throw mysql_execfail(string("CMySql::Connect: mysql_options MYSQL_OPT_CONNECT_TIMEOUT ") + oss.str() + ":" + mysql_error(_Mysql));
	}
	//���ö���ʱʱ��
	if (mysql_options(_Mysql, MYSQL_OPT_READ_TIMEOUT, (const char*)&_read_timeout_s)) {
		_errno = mysql_errno(_Mysql);
        std::ostringstream oss;
        oss << _read_timeout_s;
		throw mysql_execfail(string("CMySql::Connect: mysql_options MYSQL_OPT_READ_TIMEOUT ") + oss.str() + ":" + mysql_error(_Mysql));
	}
	//����д��ʱʱ��
	if (mysql_options(_Mysql, MYSQL_OPT_WRITE_TIMEOUT, (const char*)&_write_timeout_s)) {
		_errno = mysql_errno(_Mysql);
        std::ostringstream oss;
        oss << _write_timeout_s;
		throw mysql_execfail(string("CMySql::Connect: mysql_options MYSQL_OPT_WRITE_TIMEOUT ") + oss.str() + ":" + mysql_error(_Mysql));
	}

	/*
	   if (mysql_real_connect(_Mysql,_host.c_str(),_user.c_str(),_pass.c_str()
	   , NULL, _port, NULL, 0) == NULL)
	   throw mysql_execfail(string("CMySql::Connect: mysql_real_connect to ") + _host + ":" + mysql_error(_Mysql));
	 */
	// ֧�ִ洢���̵��� -- haritema
	if (mysql_real_connect(_Mysql, _host.c_str(), _user.c_str(), _pass.c_str()
				, NULL, _port, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
	{
		_errno = mysql_errno(_Mysql);
		throw mysql_execfail(string("CMySql::Connect: mysql_real_connect to ") + _host + ":" + mysql_error(_Mysql));
	}
	_bIsConn = true;
}

void CMySql::Close()
{
	if (!_bIsConn) return;
	mysql_close(_Mysql);
	_bIsConn = false;
}

void CMySql::Select()
{
	if (!_bIsConn) Connect();

	if (_dbname.empty()) return;

	if (mysql_select_db(_Mysql, _dbname.c_str())) {
		int ret_errno = _errno = mysql_errno(_Mysql);
		Close();
		if (ret_errno == 2013 || ret_errno == 2006) { // CR_SERVER_LOST������һ��
			Connect();
		}

		if (mysql_select_db(_Mysql, _dbname.c_str()))
			throw mysql_execfail(string("CMySql::Select: mysql_select_db ") + _dbname + ":" + mysql_error(_Mysql));
	}
}

std::string CMySql::escape_string(const char* s, size_t length)
{
	if (!_bIsConn) Connect();

	char* p = new char[length * 2 + 1];
	char* pp = p;
	pp += mysql_real_escape_string(_Mysql, pp, s, length);
	std::string s1(p, pp - p);
	delete[] p;
	return s1;
}

int CMySql::GetErrno() const
{
	return _errno;
}



