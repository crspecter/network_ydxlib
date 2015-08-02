#ifndef __YDX_TCP_CLIENT_H__
#define __YDX_TCP_CLIENT_H__

#include <boost/noncopyable.hpp>
#include <string>
#include "ydx_mutex.h"
#include "tcp_connection.h"



namespace ydx
{
class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable
{
public:
	TcpClient(EPollPoller* epoller,
		      const InetAddress& serverAddr,
		      const std::string& nameArg);
	~TcpClient();

	void connect();
	void disconnect();
	void stop();

  TcpConnectionPtr connection() const
  {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EPollPoller* getLoop() const { return epoller_; }
  bool retry() const;
  void enableRetry() { retry_ = true; }
  bool is_connected(){ return connect_ == true;}
  const std::string& name() const
  { return name_; }

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
 // void setWriteCompleteCallback(const WriteCompleteCallback& cb)
 // { writeCompleteCallback_ = cb; }	
private:
	/// Not thread safe, but in loop
	void newConnection(int sockfd);
	/// Not thread safe, but in loop
	void removeConnection(const TcpConnectionPtr& conn);

	EPollPoller* epoller_;
	ConnectorPtr connector_; // avoid revealing Connector
	const std::string name_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	bool retry_;   // atomic
	bool connect_; // atomic
	// always in loop thread
	int nextConnId_;
	mutable MutexLock mutex_;
	TcpConnectionPtr connection_; // @GuardedBy mutex_

	
};

}

#endif
