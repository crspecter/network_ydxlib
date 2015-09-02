#include "log_file.h"
#include "logging.h"

using namespace ydx;

boost::scoped_ptr<ydx::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
  g_logFile->append(msg, len);
}

void flushFunc()
{
  g_logFile->flush();
}

int main()
{
	g_logFile.reset(new LogFile);
	Logger::setOutput(outputFunc);
	Logger::setFlush(flushFunc);
	std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	for (int i = 0; i < 100000; ++i)
	{
		LOG_INFO << line << i;
		usleep(10);
	}
	return 0;
}


