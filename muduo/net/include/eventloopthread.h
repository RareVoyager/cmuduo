#include <condition_variable>
#include <mutex>

#include <include/noncopyable.h>
#include <include/thread.h>

#pragma once
namespace cmuduo
{
	namespace net
	{
		class EventLoop;
		class Thread;

		class EventLoopThread : base::noncopyable
		{
		public:
			using ThreadInitCallback = std::function<void(EventLoop*)>;
			EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback() , const std::string& name = std::string());

			~EventLoopThread();

			EventLoop* startLoop();

		private:
			void threadFunc();

		private:
			EventLoop* loop_;
			Thread thread_;
			std::mutex mutex_;
			bool exiting_;
			std::string name_;
			ThreadInitCallback callback_;
			std::condition_variable cond_;
		};
	}// namespace net

}// namespace cmuduo