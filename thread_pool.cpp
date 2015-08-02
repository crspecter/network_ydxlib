#include "thread_pool.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>


using namespace ydx;


ThreadPool::ThreadPool(const std::string& nameArg)
  : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}


void ThreadPool::stop()
{
	{
		MutexLockGuard lock(mutex_);
		running_ = false;
		notEmpty_.notifyAll();
	}
	for_each(threads_.begin(),
	       	 threads_.end(),
	         boost::bind(&ydx::Thread::join, _1));
}

void ThreadPool::start(int numThreads)
{
	
	running_ = true;
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		char id[32];
		snprintf(id, sizeof id, "%d", i+1);
		threads_.push_back(new ydx::Thread(
		      boost::bind(&ThreadPool::runInThread, this), name_+id));
		threads_[i].start();
	}
	if (numThreads == 0 && threadInitCallback_)
	{
		threadInitCallback_();
	}
}

void ThreadPool::run(const Task& task)
{

	if (threads_.empty())
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_);
		while (isFull())
		{
			notFull_.wait();
		}

		queue_.push_back(task);
		notEmpty_.notify();
	}
	
}

void ThreadPool::runInThread()
{
	try
	{
	    if (threadInitCallback_)
	    {
	    	threadInitCallback_();
	    }
		while (running_)
		{
			Task task(take());
			if (task)
			{
				task();
			}
		}
	}

	catch (...)
	{
		fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
		throw; // rethrow
	}
}


size_t ThreadPool::queueSize() const
{
	MutexLockGuard lock(mutex_);
	return queue_.size();
}


ThreadPool::Task ThreadPool::take()
{
	MutexLockGuard lock(mutex_);
	// always use a while-loop, due to spurious wakeup
	while (queue_.empty() && running_)
	{
		notEmpty_.wait();
	}
	
	Task task;
	
	if (!queue_.empty())
	{
		task = queue_.front();
		queue_.pop_front();
		if (maxQueueSize_ > 0)
		{
		  notFull_.notify();
		}
	}
	
	return task;
}

bool ThreadPool::isFull() const
{
  return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}



