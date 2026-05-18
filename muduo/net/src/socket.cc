#include <include/inetaddress.h>
#include <include/log.h>
#include <include/socket.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <unistd.h>

namespace cmuduo
{
	namespace net
	{
		Socket::Socket(int fd) : sockfd_(fd)
		{
		}

		Socket::~Socket()
		{
			::close(sockfd_);
		}

		void Socket::bindAddress(const base::InetAddress* localaddr)
		{
			if (0 != bind(sockfd_, (sockaddr*)localaddr->getSockAddr(), sizeof(sockaddr_in)))
			{
				LOG_FATAL("bind faild");
			}
		}

		void Socket::listen()
		{
			if (0 != ::listen(sockfd_, 1024))
			{
				LOG_FATAL("listen error");
			}
		}

		int Socket::accept(base::InetAddress* address)
		{
			sockaddr_in addr;
			bzero(&addr, sizeof(addr));
			socklen_t len = sizeof(addr);

			int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);

			if (connfd > 0)
			{
				address->setSockAddr(addr);
			}
			return connfd;
		}

		void Socket::shutdownWrite()
		{
			if (::shutdown(sockfd_, SHUT_WR) < 0)
			{
				LOG_ERROR("shutdownWrite error");
			}
		}

		// 协议层面
		void Socket::setTcpNoDelay(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
		}

		void Socket::setReuseAddr(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		}

		void Socket::setReusePort(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
		}

		void Socket::setKeepAlive(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
		}

	}// namespace net


}// namespace cmuduo