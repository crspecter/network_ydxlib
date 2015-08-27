#ifndef __YDX_FILE_UTIL_H__
#define __YDX_FILE_UTIL_H__

#include "string_piece.h"

#include <boost/noncopyable.hpp>

namespace ydx
{
class AppendFile : boost::noncopyable
{
 public:
  explicit AppendFile(StringArg filename);

  ~AppendFile();

  void append(const char* logline, const size_t len);

  void flush();

  size_t writtenBytes() const { return writtenBytes_; }

 private:

  size_t write(const char* logline, size_t len);

  FILE* fp_;
  char buffer_[64*1024];
  size_t writtenBytes_;
};
}

#endif
