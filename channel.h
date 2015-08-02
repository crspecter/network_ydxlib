#ifndef __YDX_CHANNEL_H__
#define __YDX_CHANNEL_H__

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace ydx
{
class EPollPoller;
class Channel : boost::noncopyable
{
public:
	typedef boost::function<void()> EventCallback;
	typedef boost::function<void()> ReadEventCallback;	

	Channel(EPollPoller* ep, int fd);
	~Channel(){}
	void handleEvent();
	void setReadCallback(const ReadEventCallback& cb)
	{ readCallback_ = cb; }
	void setWriteCallback(const EventCallback& cb)
	{ writeCallback_ = cb; }
	void setCloseCallback(const EventCallback& cb)
	{ closeCallback_ = cb; }
	void setErrorCallback(const EventCallback& cb)
	{ errorCallback_ = cb; }	


	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	bool isWriting() const { return events_ & kWriteEvent; }

  	int fd() const { return fd_; }
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }
	
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; } // used by pollers
	bool isNoneEvent() const { return events_ == kNoneEvent; }

	void remove();
	
	void update();	
	
private:
	const int  fd_;
	int        events_;
	int        revents_; // it's the received event types of epoll or poll
	int        index_; // used by Poller.

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	EPollPoller* epoller_;

private:
	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

};

}

#endif
