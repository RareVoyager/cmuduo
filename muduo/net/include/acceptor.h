#pragma once
#include <functional>

#include <include/channel.h>
#include <include/eventloop.h>
#include <include/inetaddress.h>
#include <include/noncopyable.h>
#include <include/socket.h>
namespace cmuduo
{
	namespace net
	{

		class Acceptor : base::noncopyable
		{

		public:
			using NewConnectionCallback = std::function<void(int sockfd, const base::InetAddress&)>;
			Acceptor(EventLoop* loop, const base::InetAddress& listenAddress, bool reuseport);
			~Acceptor();


			void setNewConnectionCallback(const NewConnectionCallback& cb)
			{ newConnectionCallback_ = std::move(cb); }

			bool isListen() const
			{ return listenning_; }
			void listen();


		private:
			void handleRead();
			// mainloop 也就是用户创建的loop
			EventLoop* loop_;
			Socket acceptSocket_;
			Channel acceptChannel_;
			NewConnectionCallback newConnectionCallback_;
			bool listenning_;
		};


	}// namespace net

}// namespace cmuduo