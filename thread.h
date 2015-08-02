#ifndef __YDX_THREAD_H__
#define __YDX_THREAD_H__
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <pthread.h>
#include "types.h"
#include "ydx_atomic.h"

namespace ydx
{

class Thread : boost::noncopyable
{
public:
	typedef boost::function<void ()> ThreadFunc;
	explicit Thread(const ThreadFunc&, const std::string& name = "ydxlib_thread");
	~Thread();

	void start();
	int join();


  	pid_t tid() const { return *tid_; }
	
	bool started() const 
	{
		return started_;
	}
private:
 	void setDefaultName();
	
private:
	
	bool       started_;
	bool       joined_;
	pthread_t  pthreadId_;
	boost::shared_ptr<pid_t> tid_;
	ThreadFunc func_;
	std::string     name_;

	static AtomicInt32 numCreated_;
};

}

#endif
