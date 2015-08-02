#ifndef __YDX_TCP_SERVER_H__
#define __YDX_TCP_SERVER_H__

#include "ydx_atomic.h"
#include "types.h"
#include "tcp_connection.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace ydx
{
class Acceptor;
class EPollPoller;
class EPollerThreadPool;

class TcpServer : boost::noncopyable
{
public:
	typedef boost::function<void (EPollPoller*)> ThreadInitCallback;
	enum Option
	{
		kNoReusePort,
		kReusePort,
	};
	
	TcpServer(EPollPoller* epoll,
	        const InetAddress& listenAddr,
	        const std::string& nameArg,
	        Option option = kNoReusePort);
  	~TcpServer();


  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in epoller_loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in epoller_loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
	void setThreadNum(int numThreads);
  //void setThreadInitCallback(const ThreadInitCallback& cb)
  //{ threadInitCallback_ = cb; }
	boost::shared_ptr<EPollerThreadPool> threadPool()
	{ return threadPool_; } 
	void start();

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
  //void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  //{ writeCompleteCallback_ = cb; }  
private:
	/// Not thread safe, but in loop
	void newConnection(int sockfd, const InetAddress& peerAddr);
	/// Thread safe.
	void removeConnection(const TcpConnectionPtr& conn);
	/// Not thread safe, but in loop
	//void removeConnectionInLoop(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);
	typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

	EPollPoller* epoller_;  // the acceptor loop
	const std::string hostport_;
	const std::string name_;
	boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
	boost::shared_ptr<EPollerThreadPool> threadPool_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	//WriteCompleteCallback writeCompleteCallback_;
	//ThreadInitCallback threadInitCallback_;
	AtomicInt32 started_;
	// always in loop thread
	int nextConnId_;
	ConnectionMap connections_;	
	
};

}

#endif
