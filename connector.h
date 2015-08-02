#ifndef __YDX_CONNECTOR_H__
#define __YDX_CONNECTOR_H__
#include "inet_address.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace ydx
{
class Channel;
class EPollPoller;

class Connector : boost::noncopyable,
                  public boost::enable_shared_from_this<Connector>
{

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (-1)        ///<ÎÞÐ§Ì×½Ó×Ö
#endif


public:
	typedef boost::function<void (int sockfd)> NewConnectionCallback;
	Connector(EPollPoller* ep, const InetAddress& serverAddr);
	~Connector();

	void start();  // can be called in any thread
	void restart();  // must be called in loop thread
	void stop();  // can be called in any thread

	void setNewConnectionCallback(const NewConnectionCallback cb)
	{ newConnectionCallback_ = cb ;}
	const InetAddress& serverAddress() const { return serverAddr_; }

	
private:
	enum States { kDisconnected, kConnecting, kConnected };
	static const int kMaxRetryDelayMs = 5;
	static const int kInitRetryDelayMs = 500;	
	
	void setState(States s) { state_ = s; }

	void connect();
	void connecting(int sockfd);
	////////////////////
	void loopHandleError();
	void loopConnect();
	////////////////////
	void handleWrite();
	void handleError();
	void retry(int sockfd);
	int  removeAndResetChannel();
	void resetChannel();
	
private:
	
	States state_;  // FIXME: use atomic variable	
	InetAddress serverAddr_;
	bool connect_; // atomic
	boost::scoped_ptr<Channel> channel_;
	NewConnectionCallback newConnectionCallback_;
	EPollPoller* epoller_;
	
};


}

#endif
