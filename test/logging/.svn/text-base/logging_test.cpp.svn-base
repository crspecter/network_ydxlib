#include "logging.h"
#include <string>
using namespace ydx;


int main()
{
	char buf[512];

	int n = readlink("/proc/self/exe" , buf , sizeof(buf));
	(void)n;
        std::string b = ::basename(buf);
	LOG_INFO << buf;
        LOG_INFO << b;
	LOG_INFO << "hello INFO " << 123;
	LOG_WARN << "hello WARN "  << 456;
	LOG_ERROR<< "hello ERROR " << 789;
	LOG_FATAL<< "fatal " << 963;
}
