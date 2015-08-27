#include "tcp_server.h"
#include "acceptor.h"
#include "epoller.h"
#include "epoller_thread_pool.h"
#include "socket_ops.h"

#include <boost/bind.hpp>
#include <stdio.h>
#include <iostream>
#include "logging.h"
using namespace ydx;

TcpServer::TcpServer(EPollPoller* epoll,
                     const InetAddress& listenAddr,
                     const std::string& nameArg,
                     Option option)
  : epoller_(epoll),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(epoll, listenAddr, option == kReusePort)),
    threadPool_(new EPollerThreadPool(epoll, name_)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)
{
	acceptor_->setNewConnectionCallback(
	  boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
  
  LOG_INFO << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (ConnectionMap::iterator it(connections_.begin());
      it != connections_.end(); ++it)
  {
	    TcpConnectionPtr conn = it->second;
	    it->second.reset();
	    conn->getLoop()->runInLoop(
	      boost::bind(&TcpConnection::connectDestroyed, conn));
	    conn.reset();
  }
}

void TcpServer::setThreadNum(int numThreads)
{
	threadPool_->setThreadNum(numThreads);
}


void TcpServer::start()
{
	if (started_.getAndSet(1) == 0)
	{
		threadPool_->start();
		acceptor_->listen();
	} 
}


void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
 
	EPollPoller* ioLoop = threadPool_->getNextLoop();
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);
	++nextConnId_;
	std::string connName = name_ + buf;

	LOG_INFO << "TcpServer::newConnection [" << name_
	   << "] - new connection [" << connName
	   << "] from " << peerAddr.toIpPort();
	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	TcpConnectionPtr conn(new TcpConnection(ioLoop,
	                                  connName,
	                                  sockfd,
	                                  localAddr,
	                                  peerAddr));
	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	//conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(
	boost::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
	ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn));
	
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  // FIXME: unsafe
  epoller_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();

  
  EPollPoller* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      boost::bind(&TcpConnection::connectDestroyed, conn));
  
  const std::string &str = conn->name();
  ConnectionMap::iterator it = connections_.find(str);
  it->second.reset();
  connections_.erase(it);
}