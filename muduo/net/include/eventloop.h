#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <include/currentthread.h>
#include <include/noncopyable.h>
#include <include/timestamp.h>


namespace cmuduo
{
	namespace net
	{
		class Channel;
		class Poller;

		class EventLoop : base::noncopyable
		{
		public:
			using Functor = std::function<void()>;

			EventLoop();
			~EventLoop();

			// 开启事件循环
			void loop();

			// 退出事件循环
			void quit();

			base::TimeStamp pollReturnTime() const
			{
				return pollReturnTime_;
			}

			// 当前线程执行cb
			void runInLoop(Functor cb);

			// cb放入队列,唤醒loop所在线程, 执行cb
			void queueInLoop(Functor cb);

			// 唤醒loop所在线程
			void wakeup();

			// 调用Poller 的方法
			void updateChannel(Channel* channel);
			void removeChannel(Channel* channel);
			bool hasChannel(Channel* channel);

			bool isInLoopThread() const
			{ return threadId_ == cmuduo::CurrentThread::tid(); }

		private:
			// 处理wakeup
			void handleRead();
			// 执行回调
			void doPendingFunctors();

		private:
			using ChannelList = std::vector<Channel*>;
			std::atomic_bool looping_;
			// 标识退出loop
			std::atomic_bool quit_;
			// poller 返回发生事件的channel 的时间点
			base::TimeStamp pollReturnTime_;

			const pid_t threadId_;

			// 当前EventLoop 保存的poller
			std::unique_ptr<Poller> poller_;

			// mainloop 获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理相关操作
			int wakeupFd_;
			std::unique_ptr<Channel> wakeupChannel_;
			ChannelList activeChannels_;
			// Channel* currentActiveChannle;

			// 当前loop 是否有回调操作
			std::atomic_bool callingPendingFunctors_;
			// 存储loop 需要执行的所有回调操作
			std::vector<Functor> pendingFunctor_;
			// 保护vector 线程安全
			std::mutex mutex_;
		};

	}// namespace net

}// namespace cmuduo