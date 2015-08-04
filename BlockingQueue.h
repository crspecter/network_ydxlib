#ifndef __YDX_BLOCKING_QUEUE_H__
#define __YDX_BLOCKING_QUEUE_H__


#include "ydx_mutex.h"
#include "ydx_condition.h"

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>


namespace ydx
{
template<typename T>
class BlockingQueue : boost::noncopyable
{
public:
	BlockingQueue()
	:cond_(MutexLock_),
	
	{
	}

	void put(const T& x)
	{
		MutexLockGuard lock(MutexLock_);
		queue_.push_back(x);
		cond_.notify(); // wait morphing saves us
		// http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
	}

	void put(const T&& x)
	{
		MutexLockGuard lock(MutexLock_);
		queue_.push_back(std::move(x));
		cond_.notify(); // wait morphing saves us
		// http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
	}

	T take()
	{
		MutexLockGuard lock(MutexLock_);
		while(queue_.empty())
		{
			cond_.wait();
		}
		T t(std::move(queue_.front()));
		queue_.pop_front();
		return t;
	}

	bool empty()
	{
		MutexLockGuard lock(MutexLock_);
		return queue_.empty();
	}
	
	size_t size()
	{
		MutexLockGuard lock(MutexLock_);
		return queue_.size();
	}
private:
	mutable MutexLock_;
	Condition cond_;
	std::deque<T> queue_; 

	
};

};


#endif
