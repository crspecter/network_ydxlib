#include "stream_buffer.h"

#include <string.h>
#include <stdio.h>


using namespace ydx;

int StreamBuffer::alloc(uint64_t size)
{
	data_ = ::malloc(size + 1);
	size_ = size;
	
	if(data_ == NULL)
		return -1;
	else
		return  0;	
}


void StreamBuffer::copy_in(const void *data, uint64_t len)
{
	uint64_t l;
	l = MIN(len, ((size_ + 1) - in_));
	memcpy((char*)data_ + in_, data, l);
	memcpy(data_, (char*)data + l, len - l);	
}


void StreamBuffer::copy_out(void *buf, uint64_t len)
{
	uint64_t l;
	l = MIN(len, ((size_ + 1) - out_));
	memcpy(buf, (char*)data_ + out_, l);
	memcpy((char*)buf + l, data_, len - l);
}

uint64_t StreamBuffer::copy_in_must(const void *data, uint64_t len)
{

	uint64_t l = unused();
	if( len > l || data == NULL)
		return 0;	
	
	copy_in(data, len);
	in_ = (in_ + len) % (size_ + 1);
	return len;
}


uint64_t StreamBuffer::copy_out_must(void* buf, uint64_t len)
{
		
	uint64_t l = used();
	if( len > l || buf == NULL )
		return 0;	
	
	copy_out(buf, len);
	out_ = (out_ + len) % (size_ + 1);
	return len;
}

uint64_t StreamBuffer::peek_out_must(void* buf, uint64_t len)
{
	uint64_t l;
	l = used();
	if(len > l)
	{
		return 0;
	}

	if(buf)
	{
		copy_out(buf, len);
	}
	return len;
}


bool StreamBuffer::has_writen(uint64_t len)
{
	if(len > unused())
	{
		return false;
	}

	in_ = (in_ + len) % (size_ + 1);
	return true;
}

bool StreamBuffer::has_read(uint64_t  len)
{
	if(len > used())
	 	return false;
	
	out_ = (out_ + len) % (size_ + 1);
	return true;
}


bool StreamBuffer::write_buffer(const void* buf, int len)
{
	uint64_t totb = len + sizeof(int);
	if(unused() < totb)
	{
		return false;
	}
	uint64_t inb;
	inb = copy_in_must((void *)&len, sizeof(int));
	inb += copy_in_must(buf, len);
	if(inb != totb)
	{
		printf("write_buffer error..\n");
		return false;
	}
	return true;
}


bool StreamBuffer::read_buffer(void* buf, int &len)
{
	//读取消息头部的4个字节长度，len会修正为一个消息的正确长度
	if(!peek_out_must((void *)&len, sizeof(int)))
	{
		len = 0;
		return false;
	}

	uint64_t totb = len + sizeof(int);
	if(used() < totb)
	{
		len = 0;
		return false;
	}

	uint64_t outb = 0;
	if(has_read(sizeof(int)))
		outb += sizeof(int);

	outb += copy_out_must(buf, len);
	if(outb != totb)
	{
		printf("read_buffer error..\n");
		return false;
	}

	return true;
	

	
}