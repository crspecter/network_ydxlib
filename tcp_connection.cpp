#include <boost/bind.hpp>
#include "tcp_connection.h"
#include "socket_ops.h"
#include "sockets.h"
#include "channel.h"
#include <iostream>
#include "epoller.h"
#include "logging.h"
using namespace ydx;

void ydx::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
  	LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  // do not call conn->forceClose(), because some users want to register message callback only.
}

void ydx::defaultMessageCallback(const TcpConnectionPtr&,
                                        Buffer* buf
                                        )
{
    buf->retrieve_all();
}

TcpConnection::TcpConnection(EPollPoller* ep,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : epoller_(ep),
    name_(nameArg),
    state_(kConnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(epoller_, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
    //highWaterMark_(64*1024*1024)
{
	channel_->setReadCallback(
	  boost::bind(&TcpConnection::handleRead, this));
	channel_->setWriteCallback(
	  boost::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(
	  boost::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(
	  boost::bind(&TcpConnection::handleError, this));
	//LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
	//          << " fd=" << sockfd;
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
  //LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
  //          << " fd=" << channel_->fd()
  //          << " state=" << stateToString();
  //assert(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
{
	return socket_->getTcpInfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const
{
  char buf[1024];
  buf[0] = '\0';
  socket_->getTcpInfoString(buf, sizeof buf);
  return std::string(buf);
}


void TcpConnection::send(const StringPiece& message)
{
	send(message.data(), message.size());
}

void TcpConnection::send(const void* data, int len)
{
	ssize_t nwrote = 0;
	size_t remaining = len;	
	bool faultError = false;
	//no thing in output_buffer write directly
	if(outputBuffer_.readable_bytes()== 0)
	{
		nwrote = sockets::write(channel_->fd(), data, len);
		if (nwrote >= 0)
	    {
			remaining = len - nwrote;
	    }
		else //nwrote < 0
		{
				
			nwrote = 0;
			if (errno != EWOULDBLOCK)
			{
				printf("TcpConnection::send faild\n");
				if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
				{
					faultError = true;
				}
			}
			
		}
	}

	if (!faultError && remaining > 0)	
	{
	    //size_t oldLen = outputBuffer_.readable_bytes();
	    outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
	    if (!channel_->isWriting())
	    {
	      channel_->enableWriting();
	    }

	}
}

void TcpConnection::shutdown()
{
  // FIXME: use compare and swap
  if (state_ == kConnected)
  {
    setState(kDisconnecting);
    // FIXME: shared_from_this()?
    epoller_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop()
{
  if (!channel_->isWriting())
  {
    // we are not writing
    socket_->shutdownWrite();
  }
}

const char* TcpConnection::stateToString() const
{
	switch (state_)
	{
		case kDisconnected:
		  return "kDisconnected";
		case kConnecting:
		  return "kConnecting";
		case kConnected:
		  return "kConnected";
		case kDisconnecting:
		  return "kDisconnecting";
		default:
		  return "unknown state";
	}
}

void TcpConnection::setTcpNoDelay(bool on)
{
  socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished()
{

  //assert(state_ == kConnecting);
  setState(kConnected);
  //channel_->tie(shared_from_this());
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
 
  if (state_ == kConnected)
  {
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::handleRead()
{
  
  int savedErrno = 0;
  ssize_t n = inputBuffer_.read_fd(channel_->fd(), &savedErrno);
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_);
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
	errno = savedErrno;
	//LOG_SYSERR << "TcpConnection::handleRead";
	printf("TcpConnection::handleRead error : %d\n", errno);
	handleError();
  }
}


void TcpConnection::handleWrite()
{
  //loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               outputBuffer_.readable_bytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readable_bytes() == 0)
      {
        channel_->disableWriting();

      //  if (state_ == kDisconnecting)
      //  {
      //    shutdownInLoop();
      //  }
      }
    }
    else
    {
      //LOG_SYSERR << "TcpConnection::handleWrite";
	  printf("TcpConnection::handleWrite error.. \n");
      // if (state_ == kDisconnecting)
      // {
      //   shutdownInLoop();
      // }
    }
  }
  else
  {
    //LOG_TRACE << "Connection fd = " << channel_->fd()
    //          << " is down, no more writing";
  }
}

void TcpConnection::handleClose()
{
  
  //LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == kConnected || state_ == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
  int err = sockets::get_socket_error(channel_->fd());
  char t_errnobuf[512];
  char *ret;
  ret = strerror_r(err, t_errnobuf, sizeof t_errnobuf);  
  printf("TcpConnection::handleError: %s\n", ret);
 // LOG_ERROR << "TcpConnection::handleError [" << name_
 //           << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}