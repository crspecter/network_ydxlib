#include "channel.h"
#include <sys/epoll.h>
#include "stdio.h"
#include "epoller.h"
using namespace ydx;


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(EPollPoller* ep, int fd)
  : fd_(fd),
  	events_(0),
    revents_(0),
    index_(-1), //kNew
	epoller_(ep)   
{
}

void Channel::update()
{
	epoller_->updateChannel(this);
}


void Channel::remove()
{
	epoller_->removeChannel(this);
}



void Channel::handleEvent()
{

	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
	{
		if (closeCallback_) closeCallback_();
	}

	//if (revents_ & EPOLLNVAL)
	//{
	//	printf("Channel::handle_event() POLLNVAL\n");
	//}

	if (revents_ & (EPOLLERR /* | EPOLLNVAL*/))
	{
		if (errorCallback_) 
		{
			errorCallback_();
		}
		//if (closeCallback_) 
		//{
		//	closeCallback_();
		//}
	}
	
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (readCallback_) readCallback_();
	}
	
	if (revents_ & EPOLLOUT)
	{
		if (writeCallback_) writeCallback_();
	}

}

