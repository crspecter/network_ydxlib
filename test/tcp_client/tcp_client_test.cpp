#include "epoller.h"
#include "tcp_client.h"
#include <unistd.h>
#include <iostream>
#include <boost/bind.hpp>
#include <stdio.h>
#include <string.h>
#include "logging.h"
using namespace ydx;

struct text_msg
{
	int a;
	char b;
	char c[20];
};


void onConnection(const TcpConnectionPtr& conn)
{
	  LOG_INFO << "EchoClient - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

	text_msg msg;
	msg.a = 10;
	msg.b = 8;
	snprintf(msg.c, 20, "%s", "hello server");
	  
	conn->send(&msg, sizeof msg);
}

void onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf
                           )
{
	
	text_msg msg;
	::memmove(&msg, buf->peek(), sizeof msg);

	//std::string msg(buf->retrieve_string(buf->readable_bytes()));
	printf("a=%d b=%d c=%s\n", msg.a, msg.b, msg.c);
	buf->retrieve(sizeof msg);
	conn->send(&msg, sizeof msg);
	
}

int main()
{
	EPollPoller epoller;
	InetAddress server_addr("127.0.0.1", 6666);
	TcpClient client(&epoller, server_addr, "myclient");
	client.setConnectionCallback(
		boost::bind(onConnection, _1));
	client.setMessageCallback(boost::bind(onMessage, _1, _2));
	client.connect();
	
	sleep(1);

	epoller.poll();

	return 0;
}
