#ifndef __YDX_STREAM_BUFFER_H__
#define __YDX_STREAM_BUFFER_H__
#include <stdint.h>
#include <stdlib.h>
#include <boost/noncopyable.hpp>
namespace ydx
{

#define _byte(n)     (n)                    //B
#define _kilobyte(n) ((n)*1024UL)           //KB:2��10�η�,1024 BYTE
#define _megabyte(n) ((n)*1048576UL)        //MB:2��20�η�,1048576 BYTE �� 1024KB. 
#define _gigabyte(n) ((n)*1073741824ULL)    //GB:2��30�η�,1073741824 BYTE �� 1024MB.
#define _terabyte(n) ((n)*1099511627776ULL) //TB:2��40�η�,1099511627776 BYTE �� 1024GB. 

#ifndef MIN
#define MIN(a, b) (((a)>(b))?(b):(a))
#endif

#ifndef MAX
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif


class StreamBuffer : boost::noncopyable
{
public:
	StreamBuffer()
		: in_(0),
		  out_(0),
		  size_(0),
		  data_(NULL)
	{
		
	}
	~StreamBuffer()
	{ 
		if(NULL != data_)
		{
			::free(data_);
			data_ = NULL;
		}
			
	}
	
	int alloc(uint64_t size = _megabyte(16));
	
	void clear()
	{
		in_ = out_ = 0;
	}
	
	uint64_t used()
	{
		int64_t n = in_ - out_;     
		return (n < 0)? n + size_ :  n;
	}
	
	uint64_t unused()
	{
		return size_ - used();
	}

	void copy_out(void *buf, uint64_t len);
	void copy_in(const void *data, uint64_t len);
	
	uint64_t copy_in_must(const void *data, uint64_t len);
	uint64_t copy_out_must(void* buf, uint64_t len);
	
	uint64_t peek_out_must(void* buf, uint64_t len);

public:
	bool write_buffer(const void* buf, int len);
	bool read_buffer(void* buf, int& len);
	bool has_writen(uint64_t len);
	bool has_read(uint64_t len);
	
private:
    uint64_t    in_;     ///< in  offset
    uint64_t    out_;    ///< out offset
    //uint64_t    mask;   ///< the mask of power of two sub one
    uint64_t    size_;  ///< granularity or element's size
    void*       data_;   ///< data pointer
	//__stream_info* stream_info_;
};


}

#endif
