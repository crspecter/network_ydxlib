#ifndef __YDX_EPOLLER_H__
#define __YDX_EPOLLER_H__


#include <map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <pthread.h>
#include "ydx_mutex.h"
#include "ydx_current_thread.h"
#include <boost/scoped_ptr.hpp>
struct epoll_event;

namespace ydx
{

class Channel;
class EPollPoller :  boost::noncopyable
{
public:
	typedef boost::function<void()> Functor;
	typedef std::vector<Channel*> ChannelList;
	EPollPoller();
	virtual ~EPollPoller();
	
	void enable_wakeup();
  	void runInLoop(const Functor& cb);
	void queueInLoop(const Functor& cb);
	void wakeup();
	void update(int operation, Channel* channel);
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel); 

  	void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;


	void setContext(const boost::any& context)
	{ context_ = context; }

	const boost::any& getContext() const
	{ return context_; }
	void quit();


  	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
	void poll();
	void run_one_poll();
private:
	

	void handleRead();  // waked up
	void doPendingFunctors();
	
  	const pid_t threadId_;
	
	static const int kInitEventListSize = 16;

	
	typedef std::map<int, Channel*> ChannelMap;
	ChannelMap channels_;

	Channel* currentActiveChannel_;
	ChannelList activeChannels_;
	

	int wakeupFd_;
  	boost::scoped_ptr<Channel> wakeupChannel_;		
	// unlike in TimerQueue, which is an internal class,
	// we don't expose Channel to client.
	
	boost::any context_;
	bool callingPendingFunctors_;
	typedef std::vector<struct epoll_event> EventList;
	int epollfd_;
	EventList events_;
	bool quit_;
  	MutexLock mutex_;
  	std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
};

}

#endif