#include "epoller.h"
#include "epoller_thread.h"

#include <boost/bind.hpp>


using namespace ydx;

EPollerThread::EPollerThread(const  std::string & name)
	: epoller_(NULL),
	  exiting_(false),
	  thread_(boost::bind(&EPollerThread::threadFunc, this), name),
	  mutex_(),
	  cond_(mutex_)	  
	{}

EPollerThread::~EPollerThread()
{
	exiting_ = true;
	if(NULL != epoller_)
	{
		epoller_->quit();
		thread_.join();		
	}
}

EPollPoller* EPollerThread::startEpoll()
{
	thread_.start();
	{
		MutexLockGuard lock(mutex_);
		while(epoller_ == NULL)
		{
			cond_.wait();
		}
	}
	return epoller_;
}

void EPollerThread::threadFunc()
{
	EPollPoller epoller;

	{
		MutexLockGuard lock(mutex_);
		epoller_ = &epoller ;
		cond_.notify();
	}
	epoller_->enable_wakeup();
	epoller_->poll();
	//existed epoller main loop
	epoller_ = NULL;
}
