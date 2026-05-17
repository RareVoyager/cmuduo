#include <include/inetaddress.h>

#pragma once
namespace cmuduo
{
	namespace net
	{
		class Socket
		{
		public:
			Socket(int fd);
			~Socket();

			void bindAddress(const base::InetAddress* localaddr);
			void listen();
			int accept(base::InetAddress* address);

			int fd() const
			{ return sockfd_; }
			void shutdownWrite();

			void setTcpNoDelay(bool on);
			void setReuseAddr(bool on);
			void setReusePort(bool on);
			void setKeepAlive(bool on);

		private:
			int sockfd_;
		};

	}// namespace net

}// namespace cmuduo