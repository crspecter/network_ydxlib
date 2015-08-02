
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include "ydx_current_thread.h"


using namespace ydx;

__thread int CurrentThread::t_cachedTid = 0;

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
  }
  
}

