#include "ydx_receiver.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp> 
#include <iostream>
#include <stdio.h>
#include "stream_buffer.h"
#include "tcp_server.h"
#include "epoller.h"
using namespace ydx;


Receiver::Receiver(InetAddress& serverAddr, 
						  std::string name)
	:stream_buffer_(new StreamBuffer()),
	 thread_poll_(boost::bind(&Receiver::thread_start_epoll, this, serverAddr, name)),
	 cond_(mutex_),
	 msg_count_(0)
	
{
	stream_buffer_->alloc();
}


Receiver::~Receiver()
{

}



void Receiver::onConnection(const TcpConnectionPtr& conn)
{
	  std::cout << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN") << std::endl;


}

void Receiver::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
	//StringPiece piece = buf->to_string_piece();
	//split message
	int message_len = 0;
	while(buf->readable_bytes() != 0)
	{
		MutexLockGuard lock(mutex_);
		::memcpy(&message_len, buf->peek(), sizeof(int));
		
		//socket缓冲区内存在一条不完整的消息时应该终止写入到程序内部缓冲区stream_buffer_
		if((int)(buf->readable_bytes() - sizeof(int)) < message_len)
			break;
		
		stream_buffer_->write_buffer(
				buf->peek() + sizeof(int), message_len);
		buf->retrieve(sizeof(int) + message_len);
		if(msg_count_ ++ == 0)
		{
			cond_.notify();
		}
	}	
	
}


bool Receiver::receive(void* pData, int& nSize)
{
	bool bRet = false;

	MutexLockGuard lock(mutex_);
	if(msg_count_ == 0)
	{
		cond_.wait();
	}

	bRet = stream_buffer_->read_buffer(pData, nSize);
	if(bRet)
	{
		--msg_count_; 
	}
	
	return bRet;	
}

void Receiver::start()
{
	thread_poll_.start();
}

void Receiver::thread_start_epoll(InetAddress& serverAddr, 
									   std::string &name)

{
	epoller_ = boost::make_shared<EPollPoller>();
	server_  = boost::make_shared<TcpServer>(epoller_.get(), serverAddr, name);
	
	server_->setConnectionCallback(boost::bind(&Receiver::onConnection, this, _1));
	server_->setMessageCallback(boost::bind(&Receiver::onMessage, this, _1, _2));

	server_->start();
	
	epoller_->poll();
}