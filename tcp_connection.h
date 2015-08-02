#ifndef __YDX_TCP_CONNECTION_H__
#define __YDX_TCP_CONNECTION_H__

#include "types.h"
#include "callback_types.h"
#include "buffer.h"
#include "inet_address.h"
#include "string_piece.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;
namespace ydx
{
class Channel;
class EPollPoller;
class Socket;

class TcpConnection : boost::noncopyable,
                      public boost::enable_shared_from_this<TcpConnection>
{
public:
	/// Constructs a TcpConnection with a connected sockfd
	///
	/// User should not create this object.
	TcpConnection(EPollPoller* ep,
	            const std::string& name,
	            int sockfd,
	            const InetAddress& localAddr,
	            const InetAddress& peerAddr);
	~TcpConnection();

	EPollPoller* getLoop() const { return epoller_; }
	const std::string& name() const { return name_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }
	bool connected() const { return state_ == kConnected; }
	bool disconnected() const { return state_ == kDisconnected; }
	// return true if success.
	bool getTcpInfo(struct tcp_info*) const;
	std::string getTcpInfoString() const;
	const char* stateToString() const;

	void shutdown();
	void shutdownInLoop();
	// void send(string&& message); // C++11
	void send(const void* message, int len);
	void send(const StringPiece& message);
	// void send(Buffer&& message); // C++11
	void send(Buffer* message);  // this one will swap data

	

public:
	void setTcpNoDelay(bool on);
	void setContext(const boost::any& context)
	{ context_ = context; }

	const boost::any& getContext() const
	{ return context_; }

	boost::any* getMutableContext()
	{ return &context_; }

	void setConnectionCallback(const ConnectionCallback& cb)
	{ connectionCallback_ = cb; }

	void setMessageCallback(const MessageCallback& cb)
	{ messageCallback_ = cb; }

	//void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	//{ writeCompleteCallback_ = cb; }

	//void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
	//{ highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

public:
	/// Advanced interface
	Buffer* inputBuffer()
	{ return &inputBuffer_; }

	Buffer* outputBuffer()
	{ return &outputBuffer_; }

	/// Internal use only.
	void setCloseCallback(const CloseCallback& cb)
	{ closeCallback_ = cb; }

	// called when TcpServer accepts a new connection
	void connectEstablished();   // should be called only once
	// called when TcpServer has removed me from its map
	void connectDestroyed();  // should be called only once


private:
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

private:

	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void setState(StateE s) { state_ = s; }
	
private:
	EPollPoller* epoller_;
	const std::string name_;
	StateE state_;  // FIXME: use atomic variable
	// we don't expose those classes to client.
	boost::scoped_ptr<Socket> socket_;
	boost::scoped_ptr<Channel> channel_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	//WriteCompleteCallback writeCompleteCallback_;
	//HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;
	//size_t highWaterMark_;
	Buffer inputBuffer_;
	Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
	boost::any context_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}

#endif
