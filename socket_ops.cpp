#include "socket_ops.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ydx_endian.h"

using namespace ydx;



int sockets::create_nonblock_socket()
{
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0)
  {
    printf("create socket failed: %d\n", errno);
	abort();
  }
  return sockfd;
}


int sockets::create_block_socket()
{
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0)
  {
    printf("create socket failed: %d\n", errno);
	abort();
  }
  return sockfd;
}

void sockets::bind(int sockfd, const struct sockaddr_in &addr)
{
  int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
  if (ret < 0)
  {
    printf("bind socket failed: %d\n", errno);
	abort();
  }
}
int  sockets::connect(int sockfd, const struct sockaddr_in &addr)
{
	return ::connect(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
}


void sockets::listen(int sockfd)
{
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0)
  {
    printf("bind socket failed: %d\n", errno);
	abort();
  }

}

int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0)
  {
    int savedErrno = errno;
    printf("accept socket failed: %d\n", savedErrno);
    switch (savedErrno)
    {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        {
			printf("accept unexpect error: %d\n", savedErrno);
			abort();
		}
        break;
      default:
	  		//unkown error
        {
			printf("accept unkown error: %d\n", savedErrno);
			abort();
		}
        break;
    }
  }
  return connfd;

}

ssize_t sockets::read(int sockfd, void* buf, size_t count)
{
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}
ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
  if (::close(sockfd) < 0)
  {
    printf("close socket failed: %d\n", errno);
	abort();
  }
}

//void shutdownWrite(int sockfd);

int sockets::get_socket_error(int sockfd)
{
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
}


struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
  {
    printf("error when get local addr.\n");
  }
  return localaddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    printf("error when get peer addr.\n");
  }
  return peeraddr;
}

void sockets::shutdownWrite(int sockfd)
{
  if (::shutdown(sockfd, SHUT_WR) < 0)
  {
    printf("sockets::shutdownWrite error..\n");
  }
}

void sockets::toIpPort(char* buf, size_t size,
              const struct sockaddr_in& addr)
{
	assert(size >= INET_ADDRSTRLEN);
	::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
	size_t end = ::strlen(buf);
	uint16_t port = sockets::networkToHost16(addr.sin_port);
	assert(size > end);
	snprintf(buf+end, size-end, ":%u", port);
}


void sockets::toIp(char* buf, size_t size,
          const struct sockaddr_in& addr)
{
  assert(size >= INET_ADDRSTRLEN);
  ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}
void sockets::fromIpPort(const char* ip, uint16_t port,
                  struct sockaddr_in* addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
  {
   printf("error when fromIpPort.\n");
  }
}

bool sockets::isSelfConnect(int sockfd)
{
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port
      && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}


const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
	return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

struct sockaddr_in* sockets::sockaddr_in_cast(struct sockaddr* addr)
{
	return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}
