#ifndef __YDX_CURRENT_THREAD_H__
#define __YDX_CURRENT_THREAD_H__
#include <stdint.h>

namespace ydx
{

namespace CurrentThread
{
  extern __thread int t_cachedTid;
  void cacheTid();
  inline int tid()
  {
    if (__builtin_expect(t_cachedTid == 0, 0))
    {
      cacheTid();
    }
    return t_cachedTid;
  }
}


}


#endif
