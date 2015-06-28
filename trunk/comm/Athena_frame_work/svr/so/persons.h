// **********************************************************************
// This file was generated by a TAF parser!
// TAF version 3.0.0.31 by WSRD Tencent.
// Generated from `/usr/local/resin_system.mqq.com/webapps/communication/taf/upload/yutaoli/persons.jce'
// **********************************************************************

#ifndef __PERSONS_H_
#define __PERSONS_H_

#include <map>
#include <string>
#include <vector>
#include "jce/Jce.h"
#include "pdu_header.h"
using namespace std;


struct pdu_protocol_header;

namespace PERSON
{
    struct CommRspFields : public taf::JceStructBase
    {
    public:
        static string className()
        {
            return "PERSON.CommRspFields";
        }
        static string MD5()
        {
            return "26159cf77dcb9fe1c64b57dae580ccae";
        }
        CommRspFields()
        :retcode(0),errmsg("")
        {
        }
        void resetDefautlt()
        {
            retcode = 0;
            errmsg = "";
        }
        template<typename WriterT>
        void writeTo(taf::JceOutputStream<WriterT>& _os) const
        {
            _os.write(retcode, 0);
            _os.write(errmsg, 1);
        }
        template<typename ReaderT>
        void readFrom(taf::JceInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(retcode, 0, true);
            _is.read(errmsg, 1, true);
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.display(retcode,"retcode");
            _ds.display(errmsg,"errmsg");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.displaySimple(retcode, true);
            _ds.displaySimple(errmsg, false);
            return _os;
        }
        int Encode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_len = pdu_header?*pi_buff_len:0;
                int i_ret = 0;
                if (pdu_header && (i_ret = ::pdu_header_pack1(pdu_header, pui_buff, (uint32_t *)&i_len)) != 0) { return i_ret; }

                taf::JceOutputStream<taf::BufferWriterBuff> os;
                os.setBuffer(reinterpret_cast<char *>(pui_buff + i_len), *pi_buff_len - i_len);
                this->writeTo(os);
                i_len = i_len + static_cast<int>(os.getLength());
                if (pdu_header && (i_ret = pdu_header_pack2(pui_buff, (uint32_t *)pi_buff_len, (uint32_t *)&i_len)) != 0) { return i_ret; }
                *pi_buff_len = i_len;
            }
            catch (taf::JceNotEnoughBuff & e)
            {
            	 return -100;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	 return -104;
            }
            catch (...)
            {
            	 return -1;
            }
            return 0;
        }

        int Decode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_ret = 0;
                int i_wup_len = pdu_header?*pi_buff_len:0;
                int i_end_len = 0;
                if (pdu_header && (i_ret = pdu_header_unpack(pui_buff, (uint32_t *)&i_wup_len, pdu_header, (uint32_t *)&i_end_len)) != 0) return i_ret;

                taf::JceInputStream<taf::MapBufferReader> is;
                is.setBuffer(reinterpret_cast<const char*>(pui_buff + i_wup_len), static_cast<size_t>(*pi_buff_len - i_wup_len - i_end_len));

                this->readFrom(is);
            }
            catch (taf::JceDecodeMismatch & e)
            {
            	return -101;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	return -104;
            }
            catch (taf::JceDecodeRequireNotExist & e)
            {
            	return -103;
            }
            catch (...)
            {
            	return -1;
            }
            return 0;
        }
    public:
        taf::Int32 retcode;
        std::string errmsg;
    };
    inline bool operator==(const CommRspFields&l, const CommRspFields&r)
    {
        return l.retcode == r.retcode && l.errmsg == r.errmsg;
    }
    inline bool operator!=(const CommRspFields&l, const CommRspFields&r)
    {
        return !(l == r);
    }
    int struct_CommRspFields_pack(CommRspFields * pst_struct, uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header);
    int struct_CommRspFields_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, CommRspFields * pst_struct, pdu_protocol_header * pdu_header);
    int struct_CommRspFields_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, CommRspFields * pst_struct, pdu_protocol_header * pdu_header, uint8_t * pui_mapbuff, int32_t * pi_mapbuff_len);

    struct Person : public taf::JceStructBase
    {
    public:
        static string className()
        {
            return "PERSON.Person";
        }
        static string MD5()
        {
            return "0ce3f504d0d723d531611decd8caf08d";
        }
        Person()
        :personid(0),lastname(""),firstname(""),address(""),telephone(""),city("")
        {
        }
        void resetDefautlt()
        {
            personid = 0;
            lastname = "";
            firstname = "";
            address = "";
            telephone = "";
            city = "";
        }
        template<typename WriterT>
        void writeTo(taf::JceOutputStream<WriterT>& _os) const
        {
            _os.write(personid, 0);
            _os.write(lastname, 1);
            _os.write(firstname, 2);
            _os.write(address, 3);
            _os.write(telephone, 4);
            _os.write(city, 5);
        }
        template<typename ReaderT>
        void readFrom(taf::JceInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(personid, 0, true);
            _is.read(lastname, 1, true);
            _is.read(firstname, 2, true);
            _is.read(address, 3, true);
            _is.read(telephone, 4, true);
            _is.read(city, 5, true);
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.display(personid,"personid");
            _ds.display(lastname,"lastname");
            _ds.display(firstname,"firstname");
            _ds.display(address,"address");
            _ds.display(telephone,"telephone");
            _ds.display(city,"city");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.displaySimple(personid, true);
            _ds.displaySimple(lastname, true);
            _ds.displaySimple(firstname, true);
            _ds.displaySimple(address, true);
            _ds.displaySimple(telephone, true);
            _ds.displaySimple(city, false);
            return _os;
        }
        int Encode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_len = pdu_header?*pi_buff_len:0;
                int i_ret = 0;
                if (pdu_header && (i_ret = ::pdu_header_pack1(pdu_header, pui_buff, (uint32_t *)&i_len)) != 0) { return i_ret; }

                taf::JceOutputStream<taf::BufferWriterBuff> os;
                os.setBuffer(reinterpret_cast<char *>(pui_buff + i_len), *pi_buff_len - i_len);
                this->writeTo(os);
                i_len = i_len + static_cast<int>(os.getLength());
                if (pdu_header && (i_ret = pdu_header_pack2(pui_buff, (uint32_t *)pi_buff_len, (uint32_t *)&i_len)) != 0) { return i_ret; }
                *pi_buff_len = i_len;
            }
            catch (taf::JceNotEnoughBuff & e)
            {
            	 return -100;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	 return -104;
            }
            catch (...)
            {
            	 return -1;
            }
            return 0;
        }

        int Decode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_ret = 0;
                int i_wup_len = pdu_header?*pi_buff_len:0;
                int i_end_len = 0;
                if (pdu_header && (i_ret = pdu_header_unpack(pui_buff, (uint32_t *)&i_wup_len, pdu_header, (uint32_t *)&i_end_len)) != 0) return i_ret;

                taf::JceInputStream<taf::MapBufferReader> is;
                is.setBuffer(reinterpret_cast<const char*>(pui_buff + i_wup_len), static_cast<size_t>(*pi_buff_len - i_wup_len - i_end_len));

                this->readFrom(is);
            }
            catch (taf::JceDecodeMismatch & e)
            {
            	return -101;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	return -104;
            }
            catch (taf::JceDecodeRequireNotExist & e)
            {
            	return -103;
            }
            catch (...)
            {
            	return -1;
            }
            return 0;
        }
    public:
        taf::UInt32 personid;
        std::string lastname;
        std::string firstname;
        std::string address;
        std::string telephone;
        std::string city;
    };
    inline bool operator==(const Person&l, const Person&r)
    {
        return l.personid == r.personid && l.lastname == r.lastname && l.firstname == r.firstname && l.address == r.address && l.telephone == r.telephone && l.city == r.city;
    }
    inline bool operator!=(const Person&l, const Person&r)
    {
        return !(l == r);
    }
    int struct_Person_pack(Person * pst_struct, uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header);
    int struct_Person_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, Person * pst_struct, pdu_protocol_header * pdu_header);
    int struct_Person_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, Person * pst_struct, pdu_protocol_header * pdu_header, uint8_t * pui_mapbuff, int32_t * pi_mapbuff_len);

    struct stGetPersonInfoReq : public taf::JceStructBase
    {
    public:
        static string className()
        {
            return "PERSON.stGetPersonInfoReq";
        }
        static string MD5()
        {
            return "256dddf7d3f47a709b126d25e34aac44";
        }
        stGetPersonInfoReq()
        :personid(0)
        {
        }
        void resetDefautlt()
        {
            personid = 0;
        }
        template<typename WriterT>
        void writeTo(taf::JceOutputStream<WriterT>& _os) const
        {
            _os.write(personid, 0);
        }
        template<typename ReaderT>
        void readFrom(taf::JceInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(personid, 0, true);
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.display(personid,"personid");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.displaySimple(personid, false);
            return _os;
        }
        int Encode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_len = pdu_header?*pi_buff_len:0;
                int i_ret = 0;
                if (pdu_header && (i_ret = ::pdu_header_pack1(pdu_header, pui_buff, (uint32_t *)&i_len)) != 0) { return i_ret; }

                taf::JceOutputStream<taf::BufferWriterBuff> os;
                os.setBuffer(reinterpret_cast<char *>(pui_buff + i_len), *pi_buff_len - i_len);
                this->writeTo(os);
                i_len = i_len + static_cast<int>(os.getLength());
                if (pdu_header && (i_ret = pdu_header_pack2(pui_buff, (uint32_t *)pi_buff_len, (uint32_t *)&i_len)) != 0) { return i_ret; }
                *pi_buff_len = i_len;
            }
            catch (taf::JceNotEnoughBuff & e)
            {
            	 return -100;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	 return -104;
            }
            catch (...)
            {
            	 return -1;
            }
            return 0;
        }

        int Decode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_ret = 0;
                int i_wup_len = pdu_header?*pi_buff_len:0;
                int i_end_len = 0;
                if (pdu_header && (i_ret = pdu_header_unpack(pui_buff, (uint32_t *)&i_wup_len, pdu_header, (uint32_t *)&i_end_len)) != 0) return i_ret;

                taf::JceInputStream<taf::MapBufferReader> is;
                is.setBuffer(reinterpret_cast<const char*>(pui_buff + i_wup_len), static_cast<size_t>(*pi_buff_len - i_wup_len - i_end_len));

                this->readFrom(is);
            }
            catch (taf::JceDecodeMismatch & e)
            {
            	return -101;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	return -104;
            }
            catch (taf::JceDecodeRequireNotExist & e)
            {
            	return -103;
            }
            catch (...)
            {
            	return -1;
            }
            return 0;
        }
    public:
        taf::UInt32 personid;
    };
    inline bool operator==(const stGetPersonInfoReq&l, const stGetPersonInfoReq&r)
    {
        return l.personid == r.personid;
    }
    inline bool operator!=(const stGetPersonInfoReq&l, const stGetPersonInfoReq&r)
    {
        return !(l == r);
    }
    int struct_stGetPersonInfoReq_pack(stGetPersonInfoReq * pst_struct, uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header);
    int struct_stGetPersonInfoReq_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, stGetPersonInfoReq * pst_struct, pdu_protocol_header * pdu_header);
    int struct_stGetPersonInfoReq_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, stGetPersonInfoReq * pst_struct, pdu_protocol_header * pdu_header, uint8_t * pui_mapbuff, int32_t * pi_mapbuff_len);

    struct stGetPersonInfoRsp : public taf::JceStructBase
    {
    public:
        static string className()
        {
            return "PERSON.stGetPersonInfoRsp";
        }
        static string MD5()
        {
            return "ab741e17a10e4672193c9c494681f26a";
        }
        stGetPersonInfoRsp()
        {
        }
        void resetDefautlt()
        {
        }
        template<typename WriterT>
        void writeTo(taf::JceOutputStream<WriterT>& _os) const
        {
            _os.write(base, 0);
            _os.write(person_info, 1);
        }
        template<typename ReaderT>
        void readFrom(taf::JceInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(base, 0, true);
            _is.read(person_info, 1, true);
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.display(base,"base");
            _ds.display(person_info,"person_info");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            taf::JceDisplayer _ds(_os, _level);
            _ds.displaySimple(base, true);
            _ds.displaySimple(person_info, false);
            return _os;
        }
        int Encode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_len = pdu_header?*pi_buff_len:0;
                int i_ret = 0;
                if (pdu_header && (i_ret = ::pdu_header_pack1(pdu_header, pui_buff, (uint32_t *)&i_len)) != 0) { return i_ret; }

                taf::JceOutputStream<taf::BufferWriterBuff> os;
                os.setBuffer(reinterpret_cast<char *>(pui_buff + i_len), *pi_buff_len - i_len);
                this->writeTo(os);
                i_len = i_len + static_cast<int>(os.getLength());
                if (pdu_header && (i_ret = pdu_header_pack2(pui_buff, (uint32_t *)pi_buff_len, (uint32_t *)&i_len)) != 0) { return i_ret; }
                *pi_buff_len = i_len;
            }
            catch (taf::JceNotEnoughBuff & e)
            {
            	 return -100;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	 return -104;
            }
            catch (...)
            {
            	 return -1;
            }
            return 0;
        }

        int Decode(uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header)
        {
            try
            {
                int i_ret = 0;
                int i_wup_len = pdu_header?*pi_buff_len:0;
                int i_end_len = 0;
                if (pdu_header && (i_ret = pdu_header_unpack(pui_buff, (uint32_t *)&i_wup_len, pdu_header, (uint32_t *)&i_end_len)) != 0) return i_ret;

                taf::JceInputStream<taf::MapBufferReader> is;
                is.setBuffer(reinterpret_cast<const char*>(pui_buff + i_wup_len), static_cast<size_t>(*pi_buff_len - i_wup_len - i_end_len));

                this->readFrom(is);
            }
            catch (taf::JceDecodeMismatch & e)
            {
            	return -101;
            }
            catch (taf::JceDecodeInvalidValue & e)
            {
            	return -104;
            }
            catch (taf::JceDecodeRequireNotExist & e)
            {
            	return -103;
            }
            catch (...)
            {
            	return -1;
            }
            return 0;
        }
    public:
        PERSON::CommRspFields base;
        PERSON::Person person_info;
    };
    inline bool operator==(const stGetPersonInfoRsp&l, const stGetPersonInfoRsp&r)
    {
        return l.base == r.base && l.person_info == r.person_info;
    }
    inline bool operator!=(const stGetPersonInfoRsp&l, const stGetPersonInfoRsp&r)
    {
        return !(l == r);
    }
    int struct_stGetPersonInfoRsp_pack(stGetPersonInfoRsp * pst_struct, uint8_t * pui_buff, int32_t * pi_buff_len, pdu_protocol_header * pdu_header);
    int struct_stGetPersonInfoRsp_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, stGetPersonInfoRsp * pst_struct, pdu_protocol_header * pdu_header);
    int struct_stGetPersonInfoRsp_unpack(uint8_t * pui_buff, int32_t * pi_buff_len, stGetPersonInfoRsp * pst_struct, pdu_protocol_header * pdu_header, uint8_t * pui_mapbuff, int32_t * pi_mapbuff_len);


}



#endif