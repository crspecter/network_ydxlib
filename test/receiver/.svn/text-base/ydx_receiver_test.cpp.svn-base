#include "inet_address.h"
#include "ydx_receiver.h"
#include <stdio.h>
#include <unistd.h>
using namespace ydx;

struct text_msg
{
	int a;
	char b;
	char c[20];
};


void ParseData(void* Buf, int buflen);

int main()
{
	InetAddress server_addr("127.0.0.1", 6666);
	Receiver receiver(server_addr);
	receiver.start();
	char *buf = new char[8129];
	int len = 8192;
	while(1)
	{
		receiver.receive(buf, len);
		ParseData(buf, len);
	}

	return 0;
}

void ParseData(void* Buf, int buflen)
{
	text_msg* pmsg = (text_msg*)Buf;
	{
		printf("a = %d, b = %d c = %s\n", pmsg->a, pmsg->b, pmsg->c);
	}
}