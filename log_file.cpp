#include "log_file.h"
#include "file_util.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace ydx;

LogFile::LogFile(size_t rollSize,
						const std::string& basename,
						bool threadSafe,
						int flushInterval,
						int checkEveryN)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    mutex_(threadSafe ? new MutexLock : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0)
{
	assert(basename.find('/') == std::string::npos);
	if(basename.empty())
	{
		char buf[128];
		int n = readlink("/proc/self/exe" , buf , sizeof(buf));
		(void)n;
		basename_ = ::basename(buf);
	}
		 
	rollFile();
}

LogFile::~LogFile()
{
}


void LogFile::flush()
{
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_);
		file_->flush();
	}
	else
	{
		file_->flush();
	}
}

void LogFile::append(const char* logline, int len)
{
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_);
		append_unlocked(logline, len);
	}
	else
	{
		append_unlocked(logline, len);
	}
	
}

void LogFile::append_unlocked(const char* logline, int len)
{
  file_->append(logline, len);

  if (file_->writtenBytes() > rollSize_) //一个文件最多写rollSize_字节，默认1M
  {
    rollFile();
  }
  else
  {
    ++count_;
    if (count_ >= checkEveryN_) //
    {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_) //一个文件最多写startOfPeriod_时间默认24小时
      {
        rollFile();
      }
      else if (now - lastFlush_ > flushInterval_) //超过3秒刷新一次
      {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

bool LogFile::rollFile()
{
  time_t now = 0;
  std::string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_)
  {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start; //startOfPeriod_为整天
    file_.reset(new AppendFile(filename));
    return true;
  }
  return false;
}
std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  localtime_r(now, &tm); // FIXME: localtime_r ?
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
  filename += timebuf;


  filename += ".log";

  return filename;
}
