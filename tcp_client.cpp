#include <boost/bind.hpp>
#include <stdio.h>
#include <iostream>
#include "tcp_client.h"
#include "epoller.h"
#include "tcp_connection.h"
#include "socket_ops.h"
#include "connector.h"

using namespace ydx;


TcpClient::TcpClient(EPollPoller * epoller, 
					const InetAddress & serverAddr, 
					const std::string & nameArg)

		:epoller_(epoller),
		 connector_(new Connector(epoller, serverAddr)),
		 name_(nameArg),
   	     connectionCallback_(defaultConnectionCallback),
   	     messageCallback_(defaultMessageCallback),
   	     retry_(false),
   	     connect_(true),
   	     nextConnId_(1)		 
{
	  connector_->setNewConnectionCallback(
      		boost::bind(&TcpClient::newConnection, this, _1));
}


TcpClient::~TcpClient()
{
	std::cout << "TcpClient::~TcpClient[" << name_
	       << "] - connector " << get_pointer(connector_);
	TcpConnectionPtr conn;
	bool unique = false;
	{
		MutexLockGuard lock(mutex_);
		unique = connection_.unique();
		conn = connection_;
	}
	if (conn)
	{
		if (unique)
		{
		  conn->shutdown();
		}
	
	}
	else
	{
		connector_->stop();
	}
}

void TcpClient::connect()
{
  // FIXME: check state
  std::cout << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPort() << std::endl;


  //connect_ = true;
  connector_->start();
}

void TcpClient::stop()
{
  connect_ = false;
  connector_->stop();
}

void TcpClient::disconnect()
{
  connect_ = false;

	{
		MutexLockGuard lock(mutex_);
		if (connection_)
		{
			connection_->shutdown();
		}
	}
}

void TcpClient::newConnection(int sockfd)
{
	InetAddress peerAddr(sockets::getPeerAddr(sockfd));
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
	++nextConnId_;
	std::string connName = name_ + buf;

	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	TcpConnectionPtr conn(new TcpConnection(epoller_,
	                                      connName,
	                                      sockfd,
	                                      localAddr,
	                                      peerAddr));	
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(
		boost::bind(&TcpClient::removeConnection, this, _1));
	
  	{
		MutexLockGuard lock(mutex_);
		connection_ = conn;
  	}
	
  	conn->connectEstablished();
	connect_ = true;
}


void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	{
		MutexLockGuard lock(mutex_);
		connection_.reset();
	}	
  
	conn->connectDestroyed();
	connect_ = false;	
	if (retry_ )//&& connect_)
	{
		connector_->restart();
	} 
}