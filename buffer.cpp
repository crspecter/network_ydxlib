#include "socket_ops.h"
#include "buffer.h"
#include <errno.h>
#include <sys/uio.h>

using namespace ydx;

const size_t Buffer::head_space;
const size_t Buffer::init_space;

ssize_t Buffer::read_fd(int fd, int* savedErrno)
{
	char extra_buf[65535];
	struct iovec vec[2];
	const size_t writeable = writeable_bytes();
	vec[0].iov_base = begin() + writer_index_;
	vec[0].iov_len = writeable;
	vec[1].iov_base = extra_buf;
	vec[1].iov_len = sizeof(extra_buf);

	const int iovcnt = (writeable < sizeof(extra_buf)? 2 : 1);
	const ssize_t n = sockets::readv(fd, vec, iovcnt);
	if (n < 0)
	{
		*savedErrno = errno;
	}
	else if (implicit_cast<size_t>(n) <= writeable)
	{
		writer_index_+= n;
	}	
	else
	{
		writer_index_ = buffer_.size();
		append(extra_buf, n - writeable);
	}
	return n;
}

