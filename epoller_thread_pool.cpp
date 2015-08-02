#include <boost/bind.hpp>
#include <stdio.h>

#include "epoller.h"
#include "epoller_thread.h"
#include "epoller_thread_pool.h"


using namespace ydx;

EPollerThreadPool::EPollerThreadPool(EPollPoller * epoll_base, const  std::string & nameArg)
	:epoll_base_(epoll_base),
	 name_(nameArg),
	 started_(false),
	 numThreads_(0),
	 next_(0)
	{}


EPollerThreadPool::~EPollerThreadPool()
	{}


void EPollerThreadPool::start()
{
	started_ = true;
	for (int i = 0; i < numThreads_ ; ++i)
	{
		char buf[name_.size() + 32];
    	snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
		EPollerThread* t = new EPollerThread(name_);
		threads_.push_back(t);
		epolls_.push_back(t->startEpoll());
	}
	if( numThreads_ == 0)
	{
		
	}
}

EPollPoller* EPollerThreadPool::getNextLoop()
{
	EPollPoller *epoll = epoll_base_;
	if(!epolls_.empty())
	{
		epoll = epolls_[next_];
		++next_;
		if (implicit_cast<size_t>(next_) >= epolls_.size())
		{
			next_ = 0;
		}		
	}
	return epoll;
}



EPollPoller* EPollerThreadPool::getLoopForHash(size_t hashCode)
{
  
	EPollPoller* epoll = epoll_base_;

	if (!epolls_.empty())
	{
		epoll = epolls_[hashCode % epolls_.size()];
	}
	return epoll;
}

std::vector<EPollPoller*> EPollerThreadPool::getAllLoops()
{
	if (epolls_.empty())
	{
		return std::vector<EPollPoller*>(1, epoll_base_);
	}
	else
	{
		return epolls_;
	}
}

