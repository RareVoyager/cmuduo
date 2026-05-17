#include <unordered_map>

#include <include/acceptor.h>
#include <include/callbacks.h>
#include <include/eventloop.h>
#include <include/eventloopthreadpool.h>
#include <include/inetaddress.h>
#include <include/noncopyable.h>
#pragma once
namespace cmuduo
{
	namespace net
	{
		class TcpServer : base::noncopyable
		{
		public:
			using ThreadInitCallback = std::function<void(EventLoop*)>;

			enum Option
			{
				kNoReusePort,
				kReusePort
			};

			TcpServer(EventLoop* loop, const std::string& nameArg, const base::InetAddress& listenAddr, Option option = kNoReusePort);
			~TcpServer();

			// clang-format off
            void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = std::move(cb); }

            void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = std::move(cb); }

            void setMessageCallback(const MessageCallback& cb){ messageCallback_ = std::move(cb); }
            void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = std::move(cb); }

			// clang-format on

			void setThreadNum(int startNum);
			void start();

		private:
			void newConnection(int sockfd, const base::InetAddress& peerAddr);
			void removeConnection(const TcpConnectionPtr& conn);
			void removeConnectionInLoop(const TcpConnectionPtr& conn);

			using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
			// mainloop 主要运行acceptor
			EventLoop* loop_;
			const std::string ipPort_;
			const std::string name_;

			// 运行在mainloop 接受新连接
			std::unique_ptr<Acceptor> acceptor_;
			//one loop per thread
			std::shared_ptr<EventLoopThreadPool> threadPool_;

			// 新连接回调
			ConnectionCallback connectionCallback_;
			// 读写消息回调
			MessageCallback messageCallback_;
			// 消息发送完成以后的回调
			WriteCompleteCallback writeCompleteCallback_;
			// 线程初始化回调(EventLoopThread)
			ThreadInitCallback threadInitCallback_;

			std::atomic_int started_;
			int nextConnId_;
			ConnectionMap connectionMap_;
		};

	}// namespace net

}// namespace cmuduo