#include <include/log.h>
#include <include/tcpserver.h>


namespace cmuduo
{
	namespace net
	{
		EventLoop* CheckNotNull(EventLoop* loop)
		{
			if (!loop)
			{
				LOG_FATAL("%s:%s:%d loop is null! \n", __FILE__, __FUNCTION__, __LINE__);
			}
			return loop;
		}

		TcpServer::TcpServer(EventLoop* loop, const std::string& nameArg, const base::InetAddress& listenAddr, Option option)
			: loop_(CheckNotNull(loop)),
			  ipPort_(listenAddr.toIpPort()),
			  name_(nameArg),
			  acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
			  threadPool_(new EventLoopThreadPool(loop, nameArg)),
			  nextConnId_(1),
			  started_({0})
		{
			acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
		}


		TcpServer::~TcpServer()
		{
		}

		void TcpServer::setThreadNum(int startNum)
		{
			threadPool_->setThreadNum(startNum);
		}

		void TcpServer::start()
		{
			// 防止被重复启动多次
			if (started_++ == 0)
			{
				threadPool_->start(threadInitCallback_);
				loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
			}
		}

		void TcpServer::newConnection(int sockfd, const base::InetAddress& peerAddr)
		{
		}

		void TcpServer::removeConnection(const TcpConnectionPtr& conn)
		{
		}

		void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
		{
		}


	}// namespace net


}// namespace cmuduo