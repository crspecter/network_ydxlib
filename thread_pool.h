#ifndef __YDX_THREAD_POOL_H__
#define __YDX_THREAD_POOL_H__
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <deque>

#include "ydx_condition.h"
#include "ydx_mutex.h"
#include "thread.h"
#include "types.h"

namespace ydx
{

class ThreadPool : boost::noncopyable
{
public:
	typedef boost::function<void ()> Task;	
	explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
	~ThreadPool();

	// Must be called before start().
	void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
	void setThreadInitCallback(const Task& cb)
	{ threadInitCallback_ = cb; }

	void start(int numThreads);
	void stop();

	const std::string& name() const
	{ return name_; }

	size_t queueSize() const;

	// Could block if maxQueueSize > 0
	void run(const Task& f);



private:
	bool isFull() const;
	void runInThread();
	Task take();
	
private:
	
	mutable MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;
	std::string name_;
	Task threadInitCallback_;
	boost::ptr_vector<ydx::Thread> threads_;
	std::deque<Task> queue_;
	size_t maxQueueSize_;
	bool running_;	
	
};

}


#endif
