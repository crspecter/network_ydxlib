#include "connector.h"
#include "socket_ops.h"
#include "channel.h"
#include "epoller.h"
#include <boost/bind.hpp>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "logging.h"
using namespace ydx;

const int Connector::kMaxRetryDelayMs;






Connector::Connector(EPollPoller* ep, const InetAddress& serverAddr)
  : state_(kDisconnected),
    serverAddr_(serverAddr),
    connect_(false),    
    epoller_(ep)
    //retryDelayMs_(kInitRetryDelayMs)
{
  //LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
  //LOG_DEBUG << "dtor[" << this << "]";
  //assert(!channel_);
}

void Connector::start()
{
  connect_ = true;
  connect();// FIXME: unsafe
  //loopConnect();
}

void Connector::connect()
{
  int sockfd = sockets::create_nonblock_socket();
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      //LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      printf("connect error in Connector::connect\n");
      sockets::close(sockfd);
      break;

    default:
      //LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
      printf("Unexpected error in Connector::connect\n");
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::connecting(int sockfd)
{
	setState(kConnecting);
	//assert(!channel_);
	channel_.reset(new Channel(epoller_, sockfd));
	channel_->setWriteCallback(
	  	boost::bind(&Connector::handleWrite, this)); // FIXME: unsafe
	channel_->setErrorCallback(
	  	boost::bind(&Connector::handleError, this)); // FIXME: unsafe
	//channel_->setErrorCallback(
	//  	boost::bind(&Connector::loopHandleError, this)); // FIXME: unsafe
	// channel_->tie(shared_from_this()); is not working,
	// as channel_ is not managed by shared_ptr
	channel_->enableWriting();
}

void Connector::stop()
{
	connect_ = false;
	if (state_ == kConnecting)
	{
		setState(kDisconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);
	}
}




void Connector::restart()
{
  
	setState(kDisconnected);
	connect_ = true;
	connect();
	//loopConnect();
	
}

void Connector::retry(int sockfd)
{
  sockets::close(sockfd);
  setState(kDisconnected);
  if (connect_)
  {
    //LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
    //         << " in " << retryDelayMs_ << " milliseconds. ";
    //loop_->runAfter(retryDelayMs_/1000.0,
    //                boost::bind(&Connector::startInLoop, shared_from_this()));
    //retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    sleep(kMaxRetryDelayMs);
	start();
  }
  else
  {
    //LOG_DEBUG << "do not connect";
  }
}

void Connector::resetChannel()
{
	channel_.reset();
}


int Connector::removeAndResetChannel()
{
	channel_->disableAll();
	channel_->remove();
	int sockfd = channel_->fd();
	// Can't reset channel_ here, because we are inside Channel::handleEvent
	resetChannel(); // FIXME: unsafe
	return sockfd;
}


void Connector::handleWrite()
{
  //LOG_TRACE << "Connector::handleWrite " << state_;
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::get_socket_error(sockfd);
		if (err)
		{
		  //LOG_WARN << "Connector::handleWrite - SO_ERROR = "
		  //         << err << " " << strerror_tl(err);
		  retry(sockfd);
		}
		else if (sockets::isSelfConnect(sockfd))
		{
			//LOG_WARN << "Connector::handleWrite - Self connect";
			retry(sockfd);
		}
		else
		{
			setState(kConnected);
			if (connect_)
			{
				newConnectionCallback_(sockfd);
			}
			else
			{
				sockets::close(sockfd);
			}
		}
	}
	else
	{
	// what happened?
	//assert(state_ == kDisconnected);
	}
}

void Connector::handleError()
{
  //LOG_ERROR << "Connector::handleError state=" << state_;
	printf("Connector::handleError state= %d\n", state_);
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::get_socket_error(sockfd);
		//LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
		printf("error no : %d\n", err);
		retry(sockfd);
	}
}

void Connector::loopHandleError()
{
	printf("Connector::handleError state= %d\n", state_);
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::get_socket_error(sockfd);
		//LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
		printf("error no : %d, %s\n", err, strerror_tl(err));
		retry(sockfd);
	}	
}

void Connector::loopConnect()
{
//  bool connected = false;
  int sockfd;
  for(;;)
  {
	  sockfd = sockets::create_nonblock_socket(); //if failed prog die
	  int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());
	  if(ret == INVALID_SOCKET)
	  {
	  		printf("Connector::loopConnect connect failed..\n");
			sockets::close(sockfd);
			sleep(5);
	  }
	  else 
	  {
//		connected = true;
		break;
	  }	  	
  }
  connecting(sockfd);
}






