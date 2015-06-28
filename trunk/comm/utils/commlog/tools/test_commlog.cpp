#include "commlog.h"
#include <string>

using namespace std;
using namespace Athena;

int main(int argc, char *argv[])
{
	string logFile = string("/usr/local/photo/log/") + argv[0];
	ServerLogInit(logFile.c_str(), 64*1024, 10, 8);

	int ret = 1990;
	DEBUG_LOG("hello world[%d]", ret);
	return 0;
}
