#include "inet_address.h"
#include "ydx_sender.h"
#include <stdio.h>
#include <unistd.h>
using namespace ydx;

struct text_msg
{
	int a;
	char b;
	char c[20];
};

int main()
{
	InetAddress server_addr("127.0.0.1", 6666);
	Sender sender(server_addr);
	sender.start();


	text_msg msg;
	msg.a = 123456;
	msg.b = 196;
	snprintf(msg.c, 20, "%s", "hello server");
	sender.send(&msg, sizeof(msg));
	
	msg.a = 789456;
	msg.b = 132;
	snprintf(msg.c, 20, "%s", "hello client");	
	sender.send(&msg, sizeof(msg));

	while(1)
	{
		sender.send(&msg, sizeof(msg));
		sleep(1);
	}
	return 0;
}
