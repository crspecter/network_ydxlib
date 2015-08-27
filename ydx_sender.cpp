#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include "ydx_sender.h"
#include "tcp_client.h"
#include "stream_buffer.h"
#include "epoller.h"
#include <iostream>
#include <boost/make_shared.hpp> 
#include <unistd.h>
#include "logging.h"
using namespace ydx;


Sender::Sender(InetAddress& serverAddr, 
						  std::string name )
	:stream_buffer_(new StreamBuffer()),
	 cond_(mutex_),
	 thread_epoll_(
	 		boost::bind(&Sender::thread_start_epoll, this, serverAddr, name), "sender_epoll_")	 
{
	msg_count_.get();
	stream_buffer_->alloc();
}


Sender::~Sender()
{

}

void Sender::start()
{
	thread_epoll_.start();
}



void Sender::onConnection(const TcpConnectionPtr& conn)
{
	  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
	  
}

void Sender::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf
                           )
{

}

void Sender::thread_start_epoll(InetAddress& serverAddr, std::string &name)
{
	
	epoller_ =  boost::make_shared<EPollPoller>();
	client_  =  boost::make_shared<TcpClient>(epoller_.get(), serverAddr, name);
	
	client_->setConnectionCallback(
		boost::bind(&Sender::onConnection, this, _1));
	client_->setMessageCallback(
		boost::bind(&Sender::onMessage, this, _1, _2));
	client_->enableRetry();
	client_->connect();	
	//epoller_->poll();
	epoller_->enable_wakeup();
	////////////////////////////////////////////
	int nLen = 8192;
	
	char* buf = new char[nLen]; 
	
	while(thread_epoll_.started())
	{
		epoller_->run_one_poll();
		{
			MutexLockGuard lock(mutex_);
			if(msg_count_.get()== 0)
			{
				cond_.wait();
			}		
		}

		bool read_ret = stream_buffer_->read_buffer(buf, nLen);

		if(read_ret && client_->is_connected())
		{
			//two step 1.send len 2.send buffer
			client_->connection()->send(&nLen, sizeof(nLen));
			client_->connection()->send(buf, nLen);
			msg_count_.decrement();
		}
		
	}	
	delete [] buf; 	
	
}


bool Sender::send(const void* buf, int len)
{
	
	bool bRet = stream_buffer_->write_buffer(buf, len);
	if(bRet)
	{

		MutexLockGuard lock(mutex_);
		if(msg_count_.getAndAdd(1) == 0)
		{
			msg_count_.increment();
			cond_.notify();	
		}				
	}	

	return true;
}
