#ifndef __YDX_RECEIVER_H__
#define __YDX_RECEIVER_H__
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <string>
#include "inet_address.h"
#include "ydx_condition.h"
#include "thread.h"
#include "ydx_atomic.h"
#include "callback_types.h"

namespace ydx
{

class TcpServer;
class EPollPoller;
class StreamBuffer;

class Receiver : boost::noncopyable
{
public:
	Receiver(InetAddress& serverAddr, 
						  std::string name = "ydx_receiver");
	~Receiver();

	void start();
	bool receive(void* pData, int& nSize);

	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf);	

	void thread_start_epoll(InetAddress& serverAddr, 
									   std::string &name);
	
private:
	boost::shared_ptr<EPollPoller>  epoller_;
	boost::scoped_ptr<StreamBuffer> stream_buffer_;
	boost::shared_ptr<TcpServer>    server_;

	Thread thread_poll_;
	mutable MutexLock mutex_;
	Condition cond_;
	uint32_t  msg_count_;
	
};


}


#endif
