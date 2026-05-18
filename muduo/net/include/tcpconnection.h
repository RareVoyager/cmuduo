#pragma once

#include <memory>

#include <include/callbacks.h>
#include <include/eventloop.h>
#include <include/inetaddress.h>
#include <include/noncopyable.h>
#include <include/socket.h>
#include <include/timestamp.h>


namespace cmuduo
{
	namespace net
	{
		/**
         * 运行在subloop
         * TcpServer -> Acceptor -> 新用户连接，Acceptor::accept() 拿到connfd
         * -> TcpConnection 设置回调 -> Channel -> Poller -> Channel回调操作
         */
		class TcpConnection : base::noncopyable, public std::enable_shared_from_this<TcpConnection>
		{

		public:
			TcpConnection(EventLoop* loop,
						  const std::string& nameArg,
						  int sockfd,
						  const base::InetAddress& localAddr,
						  const base::InetAddress& peerAddr);
			~TcpConnection();
			// clang-format off
            EventLoop* getLoop() { return loop_; }

			std::string name() const { return name_ ;}

            void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = std::move(cb); }
			void setMessageCallback(const MessageCallback& cb){ messageCallback_ = std::move(cb); }
			void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = std::move(cb); }
			void setCloseCallback(const CloseCallback& cb){ closeCallback_ = std::move(cb); }
			void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
            {
				highWaterMarkCallback_ = std::move(cb);
				highWaterMark_ = highWaterMark;
			}

			// clang-format on
			// 建立连接
			void connectEstablished();

			void connectDestroyed();
			void shutdown();

		private:
			enum StateE
			{
				kDisconnected,
				kConnecting,
				kConnected,
				kDisconnecting
			};
			void setState(StateE state)
			{ state_ = state; }
			void handleRead(base::TimeStamp reciveTime);
			void handleWrite();
			void handleClose();
			void handleError();


			void shutdownInLoop();

			void send(const std::string& buf);

			void sendInLoop(const void* data, size_t len);

		private:
			// 不是mainloop
			EventLoop* loop_;
			const std::string name_;
			std::atomic_int state_;
			bool reading_;

			std::unique_ptr<Socket> socket_;
			std::unique_ptr<Channel> channel_;

			const base::InetAddress localAddr_;
			const base::InetAddress peerAddr_;

			// 新连接回调
			ConnectionCallback connectionCallback_;
			// 读写消息回调
			MessageCallback messageCallback_;
			// 消息发送完成以后的回调
			WriteCompleteCallback writeCompleteCallback_;
			// 关闭连接回调
			CloseCallback closeCallback_;
			// 高水位回调
			HighWaterMarkCallback highWaterMarkCallback_;
			size_t highWaterMark_;

			// 接受数据的缓冲区
			Buffer inputBuffer_;
			// 发送数据的缓冲区
			Buffer outputBuffer_;
		};

	}// namespace net


}// namespace cmuduo
