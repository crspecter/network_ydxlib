#ifndef __YDX_EPOLLER_THREAD_H__
#define __YDX_EPOLLER_THREAD_H__

#include "ydx_condition.h"
#include "ydx_mutex.h"
#include "thread.h"

#include <boost/noncopyable.hpp>

namespace ydx
{

class EPollPoller;
class EPollerThread : boost::noncopyable
{
public:
	EPollerThread( const std::string& name = std::string("epoller_thread"));
	~EPollerThread();
	EPollPoller* startEpoll();


private:
	void threadFunc();

	EPollPoller* epoller_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	

};

}


#endif
