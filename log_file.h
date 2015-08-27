#ifndef __YDX_LOG_FILE_H__
#define __YDX_LOG_FILE_H__


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include "ydx_mutex.h"


namespace ydx
{

class AppendFile;

class LogFile : boost::noncopyable
{
 public:
  LogFile(
          size_t rollSize = 1024 * 1024 * 1,
          const std::string& basename ="",
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 10000);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool rollFile();

 private:
  void append_unlocked(const char* logline, int len);

  static std::string getLogFileName(const std::string& basename, time_t* now);

  std::string basename_;
  const size_t rollSize_;
  const int flushInterval_;
  const int checkEveryN_;

  int count_;

  boost::scoped_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  boost::scoped_ptr<AppendFile> file_;

  const static int kRollPerSeconds_ = 60*60*24;
};
}



#endif
