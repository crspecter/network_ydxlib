#include "logging.h"
#include <string.h>


namespace ydx
{
__thread char t_errnobuf[512];
__thread char t_time[32];
__thread time_t t_lastSecond;

const char* strerror_tl(int err_)
{
	return strerror_r(err_, t_errnobuf, sizeof(t_errnobuf));
}

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
	"[TRACE]",
	"[DEBUG]",
	"[INFO]",
	"[WARN]",
	"[ERROR]",
	"[FATAL]",
};

void defaultOutput(const char* msg, int len)
{
  size_t n = fwrite(msg, 1, len, stdout);
  //FIXME check n
  (void)n;
}

void defaultFlush()
{
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;


Logger::LogLevel initLogLevel()
{

	return Logger::INFO;
}

Logger::LogLevel g_logLevel = Logger::INFO;
}
using namespace ydx;

Logger::Logger(SourceFile file, int line)
  : ctime_(::time(NULL)),
  	stream_(),
  	basename_(file)
  	
{
	level_ = INFO;
	errno_ = 0;
	
	line_ = line;
	stream_ << ctime_.Format(t_time);
	stream_ << LogLevelName[level_];	
	
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : ctime_(::time(NULL)),
  	stream_(),
  	basename_(file)
{
	stream_ << func << ' ';
	level_ = level;
	line_ = line;
	stream_ << ctime_.Format(t_time);
	stream_ << LogLevelName[level_];	
		
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : ctime_(::time(NULL)),
  	stream_(),
  	basename_(file)
{	
	level_ = level;
	line_ = line;
	stream_ << ctime_.Format(t_time);
	stream_ << LogLevelName[level_];	
	
}

Logger::Logger(SourceFile file, int line, bool toAbort)
  : ctime_(::time(NULL)),
  	stream_(),
  	basename_(file)
 
{	
	level_ = toAbort?FATAL:ERROR;
	line_ = line;
	stream_ << ctime_.Format(t_time);
	stream_ << LogLevelName[level_];		
}

Logger::~Logger()
{
	stream_ << " - " << StringPiece(basename_.data_, basename_.size_) << ':' << line_ << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	g_output(buf.data(), buf.length());
	if (level_ == FATAL)
	{
		g_flush();
		abort();
	}
}


void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}




