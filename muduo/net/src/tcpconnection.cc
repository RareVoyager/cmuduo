#include <errno.h>
#include <fcntl.h>// for open
#include <functional>
#include <netinet/tcp.h>
#include <string.h>
#include <string>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>// for close


#include <include/channel.h>
#include <include/log.h>
#include <include/tcpconnection.h>


namespace cmuduo
{
	namespace net
	{
		EventLoop* CheckEventLoopNotNull(EventLoop* loop)
		{
			if (!loop)
			{
				LOG_FATAL("%s:%s:%d TcpConnection loop is null! \n", __FILE__, __FUNCTION__, __LINE__);
			}
			return loop;
		}

		TcpConnection::TcpConnection(EventLoop* loop, const std::string& nameArg, int sockfd, const base::InetAddress& localAddr, const base::InetAddress& peerAddr)
			: loop_(CheckEventLoopNotNull(loop)),
			  name_(nameArg),
			  state_(kConnecting),
			  socket_(new Socket(sockfd)),
			  channel_(new Channel(loop, sockfd)),
			  localAddr_(localAddr),
			  peerAddr_(peerAddr),
			  highWaterMark_(64 * 1024 * 1024)// 64M
		{
			channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
			channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
			channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
			channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

			LOG_INFO("TcpConnection::ctor[%s] at fd=%d", name_.c_str(), sockfd);
			socket_->setKeepAlive(true);
		}

		TcpConnection::~TcpConnection()
		{
			LOG_INFO("TcpConnection dtor[%s] at fd = %d,state = %d", name_.c_str(), channel_->fd(), state_.load());
		}

		void TcpConnection::handleRead(base::TimeStamp reciveTime)
		{
			int saveErrno = 0;
			ssize_t n = inputBuffer_.readFd(socket_->fd(), &saveErrno);

			if (n > 0)
			{
				messageCallback_(shared_from_this(), &inputBuffer_, reciveTime);
			}
			else if (n == 0)
			{
				handleClose();
			}
			else
			{
				errno = saveErrno;
				LOG_ERROR("TcpConnection::handleRead");
				handleError();
			}
		}

		void TcpConnection::handleWrite()
		{
			if (channel_->isWriting())
			{
				int saveErrno = 0;
				ssize_t n = outputBuffer_.writeFd(socket_->fd(), &saveErrno);
				if (n > 0)
				{
					outputBuffer_.retrieve(n);
					// 缓冲区中没有数据了
					if (outputBuffer_.readableBytes() == 0)
					{
						// channel_ 对写不感兴趣
						channel_->disableWriting();
						// 调用写完成回调操作
						if (writeCompleteCallback_)
						{
							loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
						}

						if (state_ == kDisconnecting)
						{
							shutdownInLoop();
						}
					}
					else
					{
						LOG_ERROR("TcpConnection::handleWrite");
					}
				}
			}
			else
			{
				LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
			}
		}

		void TcpConnection::handleClose()
		{
			LOG_INFO("TcpConnection::handleClose fd=%d state=%d\n", channel_->fd(), (int)state_);
			setState(kDisconnected);
			channel_->disableAll();

			TcpConnectionPtr connPtr(shared_from_this());
			connectionCallback_(connPtr);// 连接回调
			closeCallback_(connPtr);     // 执行关闭连接的回调 执行的是TcpServer::removeConnection回调方法   // must be the last line
		}

		void TcpConnection::handleError()
		{
			int optval;
			socklen_t optlen = sizeof optval;
			int err = 0;
			if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
			{
				err = errno;
			}
			else
			{
				err = optval;
			}
			LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
		}


		void TcpConnection::send(const std::string& buf)
		{
			if (state_ == kConnected)
			{
				if (loop_->isInLoopThread())
				{
					sendInLoop(buf.c_str(), buf.size());
				}
				else
				{
					loop_->runInLoop(
							std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
				}
			}
		}

		void TcpConnection::sendInLoop(const void* data, size_t len)
		{
			ssize_t nwrote = 0;
			// 未发送完的数据
			size_t remaining = len;
			bool faultError = false;
			// 执行过shutdown
			if (state_ == kDisconnecting)
			{
				LOG_ERROR("");
				return;
			}
			// 还没向outputbuffer输出数据
			if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
			{
				nwrote = ::write(channel_->fd(), data, len);
				if (nwrote >= 0)
				{
					remaining = len - nwrote;
					// 数据发送完了并且有写回调函数，在loop_ 中执行这个函数
					if (remaining == 0 && writeCompleteCallback_)
					{
						// 在这里全部发送完成, 因此不再设置channel_ 的epollout事件
						loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
					}
				}
				else
				{
					nwrote = 0;
					if (errno != EWOULDBLOCK)
					{
						LOG_ERROR("TcpConnection::sendInLoop");
						if (errno == EPIPE || errno == ECONNRESET)
						{
							faultError = true;
						}
					}
				}
			}

			// 数据没有完全发送完成 数据需要保存到缓冲区
			// 要给channel注册epollout事件 poller(LT) 会发送缓冲区有空间通知响应的sock - channel
			// 调用handleWrite() 回调方法 把数据全部发送完成
			if (!faultError && remaining > 0)
			{
				size_t oldLen = outputBuffer_.readableBytes();
				if (oldLen + remaining > highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
				{
					loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
				}
				outputBuffer_.append((char*)data + nwrote, remaining);
				if (!channel_->isWriting())
				{
					// 注册channel的写事件，否则epoll 不会驱动epollout
					channel_->enableWriting();
				}
			}
		}

		// 建立连接
		void TcpConnection::connectEstablished()
		{
			setState(kConnected);
			channel_->tie(shared_from_this());
			// 设置对epollin感兴趣
			channel_->enableReading();
			connectionCallback_(shared_from_this());
		}
		void TcpConnection::connectDestroyed()
		{
			if (state_ == kConnected)
			{
				setState(kDisconnected);
				channel_->disableAll();
				connectionCallback_(shared_from_this());
			}
			channel_->remove();
		}

		void TcpConnection::shutdown()
		{
			if (state_ == kConnected)
			{
				setState(kDisconnecting);
				loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
			}
		}

		void TcpConnection::shutdownInLoop()
		{
			// 当前outputbuffer_ 的数据已经发送完成
			if (!channel_->isWriting())
			{
				// 触发closeCallback
				socket_->shutdownWrite();
			}
		}


	}// namespace net


}// namespace cmuduo
