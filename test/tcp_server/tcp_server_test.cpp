#include "tcp_server.h"

#include "epoller.h"


#include <arpa/inet.h>
#include <boost/bind.hpp>
#include <iostream>

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
	  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf
                           )
{
	//std::string msg(buf->retrieve_string(buf->readable_bytes()));
	//std::cout << msg << std::endl;
	//conn->send(msg);
	text_msg msg;
	::memmove(&msg, buf->peek(), sizeof msg);

	//std::string msg(buf->retrieve_string(buf->readable_bytes()));
	printf("a=%d b=%d c=%s\n", msg.a, msg.b, msg.c);
	buf->retrieve(sizeof msg);
	conn->send(&msg, sizeof msg);

}


int main()
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(6666);
	EPollPoller epoller;

	
	
	TcpServer server(&epoller, addr, "sever_test");
	server.setThreadNum(3);
	server.setConnectionCallback(
		boost::bind(onConnection, _1));
	server.setMessageCallback(
		boost::bind(onMessage, _1, _2));
	
	server.start();
	epoller.poll();
	
	return 0;
}
