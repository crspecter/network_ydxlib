#ifndef __YDX_LOGGING_H__
#define __YDX_LOGGING_H__

#include "log_stream.h"
#include "timestamp.h"

namespace ydx
{



class SourceFile
{
public:
	template<int N>
	inline SourceFile(const char (&arr)[N])
	  : data_(arr),
	    size_(N-1)
	{
		const char* slash = strrchr(data_, '/'); // builtin function
		if (slash)
		{
		data_ = slash + 1;
		size_ -= static_cast<int>(data_ - arr);
		}
	}

explicit SourceFile(const char* filename)
  : data_(filename)
{
	const char* slash = strrchr(filename, '/');
	if (slash)
	{
	data_ = slash + 1;
	}
	size_ = static_cast<int>(strlen(data_));
}

	const char* data_;
	int size_;
};



class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,
	};

	

	Logger(SourceFile file, int line);
	Logger(SourceFile file, int line, LogLevel level);
	Logger(SourceFile file, int line, LogLevel level, const char* func);
	Logger(SourceFile file, int line, bool toAbort);
	~Logger();
	
	LogStream& stream() { return stream_; }

	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);

	typedef void (*OutputFunc)(const char* msg, int len);
	typedef void (*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);
	
	void formatTime();
private:
	
	CTime ctime_;
	LogStream stream_;
	SourceFile basename_;
	LogLevel level_;
	int errno_;
	int line_;	
	
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
	return g_logLevel;
}




#define LOG_INFO if (ydx::Logger::logLevel() <= ydx::Logger::INFO) \
  ydx::Logger(__FILE__, __LINE__).stream()
  
#define LOG_WARN ydx::Logger(__FILE__, __LINE__, ydx::Logger::WARN).stream()
#define LOG_ERROR ydx::Logger(__FILE__, __LINE__, ydx::Logger::ERROR).stream()
#define LOG_FATAL ydx::Logger(__FILE__, __LINE__, ydx::Logger::FATAL).stream()
#define LOG_SYSERR ydx::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL ydx::Logger(__FILE__, __LINE__, true).stream()


const char* strerror_tl(int err_);



}

#endif
