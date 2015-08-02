#ifndef __YDX_BUFFER_H__
#define __YDX_BUFFER_H__
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string>
#include <string.h>
#include "ydx_endian.h"
#include "types.h"
#include "string_piece.h"

namespace ydx
{
class Buffer 
{
public:
	static const size_t head_space = 0;
	static const size_t init_space = 2048;
	
explicit Buffer(size_t init_size = init_space)
	: buffer_(head_space + init_size),
	  reader_index_(head_space),
	  writer_index_(head_space)	 
	{
	}

void swap(Buffer& rhs)
{
	buffer_.swap(rhs.buffer_);
	std::swap(reader_index_, rhs.reader_index_);
	std::swap(writer_index_, rhs.writer_index_);
}

size_t readable_bytes() const
{
	return writer_index_ - reader_index_;
}

size_t writeable_bytes() const
{
	return buffer_.size() - writer_index_;
}

size_t prepend_bytes() const
{
	return reader_index_;
}

const char* peek() const
{
	return begin() + reader_index_;
}

void retrieve(size_t len)
{
	if(len < readable_bytes())
	{
		reader_index_ += len;
	}
	else
	{
		retrieve_all();
	}
}

std::string retrieve_string(size_t len)
{
	std::string ret(peek(), len);
	retrieve(len);
	return ret;
}

StringPiece to_string_piece() const
{
	return StringPiece(peek(), static_cast<int>(readable_bytes()));
}



void has_written(size_t len)
{
  assert(len <= writeable_bytes());
  writer_index_+= len;
}

void unwrite(size_t len)
{
  assert(len <= readable_bytes());
  writer_index_-= len;
}

	
void retrieve_all()
{
	reader_index_ = head_space;
	writer_index_ = head_space;
}

void ensure_writeable_bytes(size_t len)
{
	if(writeable_bytes() < len)
	{
		make_space(len);
	}
}

void append(const StringPiece& str)
{
	append(str.data(), str.size());
}

void append(const void* data, size_t len)
{
	append(static_cast<const char*>(data), len);
}

void append(const char* data, size_t len)
{
	ensure_writeable_bytes(len);
	std::copy(data, data + len, begin_write());
	has_written(len);
}

char* begin_write()
{ return begin() + writer_index_; }

const char* begin_write() const
{ return begin() + writer_index_; }

void shrink(size_t reserve)
{
	Buffer other;
	other.ensure_writeable_bytes(readable_bytes() + reserve);
	other.append(to_string_piece());
	swap(other);
}

ssize_t read_fd(int fd, int* savedErrno);

private:
char* begin()
{ return &*buffer_.begin();}

const char* begin() const
{ return &*buffer_.begin();}

void make_space(size_t len)
{
	if(writeable_bytes() + prepend_bytes() < len + head_space)
	{
		buffer_.resize(writer_index_ + len);
	}
	else
	{
		size_t readable_len = readable_bytes();
		std::copy(begin() + reader_index_,
			      begin() + writer_index_,
			      begin() + head_space);
		reader_index_ = head_space;
		writer_index_ = reader_index_ + readable_len;
	}
}


private:
	std::vector<char> buffer_;
	size_t reader_index_;
	size_t writer_index_;
	
};



}


#endif
