#ifndef __YDX_SOCKET_OPTS_H__
#define __YDX_SOCKET_OPTS_H__

#include <arpa/inet.h>

namespace ydx
{

namespace sockets
{

int create_nonblock_socket();
int create_block_socket();
void bind(int sockfd, const struct sockaddr_in &addr);
int  connect(int sockfd, const struct sockaddr_in &addr);
void listen(int sockfd);

int accept(int sockfd, struct sockaddr_in* addr);
ssize_t read(int sockfd, void* buf, size_t len);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void shutdownWrite(int sockfd);
void close(int sockfd);

//void shutdownWrite(int sockfd);

int get_socket_error(int sockfd);

void toIpPort(char* buf, size_t size,
              const struct sockaddr_in& addr);
void toIp(char* buf, size_t size,
          const struct sockaddr_in& addr);
void fromIpPort(const char* ip, uint16_t port,
                  struct sockaddr_in* addr);

bool isSelfConnect(int sockfd);

struct sockaddr_in getLocalAddr(int sockfd);

struct sockaddr_in getPeerAddr(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);

}
}


#endif
