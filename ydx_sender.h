#ifndef __YDX_SENDER_H__
#define __YDX_SENDER_H__

#include <boost/scoped_ptr.hpp>
#include <string>
#include "inet_address.h"
#include "ydx_condition.h"
#include "thread.h"
#include "ydx_atomic.h"
#include "callback_types.h"
namespace ydx
{

class EPollPoller;
class StreamBuffer;
class TcpClient;

class Sender
{
public:
	Sender(InetAddress& serverAddr, 
						  std::string name = "ydx_sender");
	~Sender();
	
	void start();
	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

	//void thread_send_func();
	void thread_start_epoll(InetAddress& serverAddr, std::string &name);
	
	bool send(const void* buf, int len);



	boost::shared_ptr<EPollPoller>  epoller_;
	boost::scoped_ptr<StreamBuffer> stream_buffer_;
	boost::shared_ptr<TcpClient>    client_;
	
	mutable MutexLock mutex_;
	Condition cond_;
	Thread thread_epoll_;
	AtomicInt32 msg_count_;

};

}

#endif
