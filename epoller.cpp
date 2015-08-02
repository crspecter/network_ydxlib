#include "epoller.h"
#include "channel.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include "stdio.h"
#include "unistd.h"
#include "types.h"
#include <iostream>
#include <signal.h>
#include <sys/eventfd.h>
#include <boost/bind.hpp>
#include "socket_ops.h"

using namespace ydx;

namespace
{


const int kPollTimeMs = 10000;

int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		std::cout << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

	const int kNew = -1;
	const int kAdded = 1;
	const int kDeleted = 2;

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}





EPollPoller::EPollPoller()
  : threadId_(CurrentThread::tid()),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize),
    quit_(false)
{
	if (epollfd_ < 0)
	{
		printf("create epoller failed...\n");
		abort();
	}
}

EPollPoller::~EPollPoller()
{
  ::close(epollfd_);
}

void EPollPoller::enable_wakeup()
{

	wakeupChannel_->setReadCallback(
	  boost::bind(&EPollPoller::handleRead, this));
	// we are always reading the wakeupfd
	wakeupChannel_->enableReading();
	
}


void EPollPoller::poll()
{
	
	while(!quit_)
	{
		activeChannels_.clear();
		
		int numEvents = ::epoll_wait(epollfd_,
		                       &*events_.begin(),
		                       static_cast<int>(events_.size()),
		                       500);  //max numEvents is events_.size()
		 
		int savedErrno = errno;
		if (numEvents > 0)
		{
			
			fillActiveChannels(numEvents, &activeChannels_);
			
			if (implicit_cast<size_t>(numEvents) == events_.size())
			{
				events_.resize(events_.size()*2);  //where max events happen resize the events_
			}

		    for (ChannelList::iterator it = activeChannels_.begin();
		        it != activeChannels_.end(); ++it)
		    {
		      currentActiveChannel_ = *it;
		      currentActiveChannel_->handleEvent();
		    }	

			doPendingFunctors();

		}
		else if (numEvents == 0)
		{
			//LOG_TRACE << "nothing happended";
		}
		else
		{
			// error happens, log uncommon ones
			if (savedErrno != EINTR)
			{
				errno = savedErrno;
				//LOG_SYSERR << "EPollPoller::poll()";
			}
		}
	}
}


void EPollPoller::run_one_poll()
{
	
	if(!quit_)
	{
		activeChannels_.clear();
		
		int numEvents = ::epoll_wait(epollfd_,
		                       &*events_.begin(),
		                       static_cast<int>(events_.size()),
		                       500);  //max numEvents is events_.size()
		 
		int savedErrno = errno;
		if (numEvents > 0)
		{
			
			fillActiveChannels(numEvents, &activeChannels_);
			
			if (implicit_cast<size_t>(numEvents) == events_.size())
			{
				events_.resize(events_.size()*2);  //where max events happen resize the events_
			}

		    for (ChannelList::iterator it = activeChannels_.begin();
		        it != activeChannels_.end(); ++it)
		    {
		      currentActiveChannel_ = *it;
		      currentActiveChannel_->handleEvent();
		    }	

			doPendingFunctors();

		}
		else if (numEvents == 0)
		{
			//LOG_TRACE << "nothing happended";
		}
		else
		{
			// error happens, log uncommon ones
			if (savedErrno != EINTR)
			{
				errno = savedErrno;
				//LOG_SYSERR << "EPollPoller::poll()";
			}
		}
	}
}



void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const
{
	for (int i = 0; i < numEvents; ++i)
	{
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}

void EPollPoller::updateChannel(Channel* channel)
{

  const int index = channel->index();
	if (index == kNew || index == kDeleted)
	{
		// a new one, add with EPOLL_CTL_ADD
		int fd = channel->fd();
		if (index == kNew)
		{
			channels_[fd] = channel;
		}
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		// update existing one with EPOLL_CTL_MOD/DEL
		int fd = channel->fd();
		(void)fd;
		//assert(channels_.find(fd) != channels_.end());
		//assert(channels_[fd] == channel);
		//assert(index == kAdded);
		if (channel->isNoneEvent())
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else
		{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}



void EPollPoller::removeChannel(Channel* channel)
{

	int fd = channel->fd();
	//LOG_TRACE << "fd = " << fd;
	//assert(channels_.find(fd) != channels_.end());
	//assert(channels_[fd] == channel);
	//assert(channel->isNoneEvent());
	int index = channel->index();
	//assert(index == kAdded || index == kDeleted);
	size_t n = channels_.erase(fd);
	(void)n;
	//assert(n == 1);

	if (index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}


void EPollPoller::update(int operation, Channel* channel)
{
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  //LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
  //  << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
  {
    if (operation == EPOLL_CTL_DEL)
    {
      printf("update epoll in delete \n");
      //LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
    }
    else
    {
      printf("update epoll in modify \n");
      //LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
    }
  }
}


void EPollPoller::wakeup()
{
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
		std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8"<<std::endl;
	}
}

void EPollPoller::runInLoop(const Functor& cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(cb);
	}
}


void EPollPoller::queueInLoop(const Functor& cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(cb);
	}

	if (!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}


void EPollPoller::handleRead()
{
	uint64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
		std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" <<std::endl;
	}
}


void EPollPoller::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
		//release the lock, at this this other thread can push_pack new vector
	}

	for (size_t i = 0; i < functors.size(); ++i)
	{
		functors[i]();
	}
	callingPendingFunctors_ = false;
}


void EPollPoller::quit()
{
	quit_ = true;
	if (!isInLoopThread())
	{
	    wakeup();
	}
}
