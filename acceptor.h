#ifndef __YDX_ACCEPTOR_H__
#define __YDX_ACCEPTOR_H__

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "channel.h"
#include "sockets.h"

namespace ydx
{
class EPollPoller;
class InetAddress;

class Acceptor : boost::noncopyable
{
public:
	typedef boost::function<void (int sockfd, 
							const InetAddress)> NewConnectionCallback;

	Acceptor(EPollPoller* ep, const InetAddress& listenAddr, bool reuseport);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{ newConnectionCallback_ = cb; }

	bool listenning() const { return listenning_; }
	void listen();
	
private:
	void handleRead();
private:
	
	EPollPoller* epoller_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;
	bool listenning_;
	int idleFd_;
	
};


}



#endif

