
#include "acceptor.h"
#include <include/acceptor.h>
#include <include/log.h>

static int createNonblocking()
{
	int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (sockfd < 0)
	{
		LOG_FATAL("%s%s%d ,create socketfd error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
	}
	return sockfd;
}

namespace cmuduo
{
	namespace net
	{
		// acceptSocket_ 创建socket + bind
		Acceptor::Acceptor(EventLoop* loop, const base::InetAddress& listenAddress, bool reuseport)
			: loop_(loop),
			  acceptSocket_(createNonblocking()),
			  acceptChannel_(loop, acceptSocket_.fd()),
			  listenning_(false)
		{
			acceptSocket_.setReuseAddr(true);
			acceptSocket_.setReusePort(true);
			// bind
			acceptSocket_.bindAddress(&listenAddress);
			// readCallback 有事情发生了 代表着有新用户连接了
			acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
		}

		Acceptor::~Acceptor()
		{
			acceptChannel_.disableAll();
			acceptChannel_.remove();
		}

		void Acceptor::listen()
		{
			listenning_ = true;
			acceptSocket_.listen();
		}


		void Acceptor::handleRead()
		{
			base::InetAddress peerAddress;
			int connfd = acceptSocket_.accept(&peerAddress);
			if (connfd >= 0)
			{
				if (newConnectionCallback_)
				{
					newConnectionCallback_(connfd, peerAddress);
				}
				else
				{
					::close(connfd);
				}
			}
			else
			{
				LOG_ERROR("%s%s%d ,accept error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
				if (errno == EMFILE)
				{
					LOG_ERROR("%s:%s:%d sockfd reached limit! \n", __FILE__, __FUNCTION__, __LINE__);
				}
			}
		}
	}// namespace net

}// namespace cmuduo