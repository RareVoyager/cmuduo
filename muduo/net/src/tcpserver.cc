#include <cstring>

#include <include/log.h>
#include <include/tcpconnection.h>
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
			for (auto& item: connectionMap_)
			{
				TcpConnectionPtr conn(item.second);
				item.second.reset();
				conn->getLoop()->runInLoop([conn]() {
					conn->connectDestroyed();
				});
			}
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
			// 轮询算法获得
			EventLoop* ioLoop = threadPool_->getNextLoop();
			char buf[64] = {0};
			snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_++);
			std::string connName = name_ + buf;

			LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
					 name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

			sockaddr_in local;
			::bzero(&local, sizeof local);
			socklen_t len = sizeof local;
			if (::getsockname(sockfd, (sockaddr*)&local, &len) < 0)
			{
				LOG_ERROR("socket::getLocalAddr");
			}
			base::InetAddress localAddr(local);
			TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
			connectionMap_[connName] = conn;

			conn->setMessageCallback(messageCallback_);
			conn->setWriteCompleteCallback(writeCompleteCallback_);
			conn->setConnectionCallback(connectionCallback_);
			conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

			//ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
			ioLoop->runInLoop([conn]() { conn->connectEstablished(); });
		}

		void TcpServer::removeConnection(const TcpConnectionPtr& conn)
		{
			loop_->runInLoop([this, conn]() { removeConnectionInLoop(conn); });
		}

		void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
		{
			LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n",
					 name_.c_str(), conn->name().c_str());
			connectionMap_.erase(conn->name());
			conn->getLoop()->queueInLoop([conn]() {
				conn->connectDestroyed();
			});
		}


	}// namespace net


}// namespace cmuduo