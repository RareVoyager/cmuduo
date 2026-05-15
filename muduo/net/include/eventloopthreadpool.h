#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <include/noncopyable.h>


#pragma once
namespace cmuduo
{
	namespace net
	{
		class EventLoop;
		class EventLoopThread;
		class EventLoopThreadPool : base::noncopyable
		{
		public:
			using ThreadInitCallback = std::function<void(EventLoop*)>;
			EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
			~EventLoopThreadPool();


			void start(const ThreadInitCallback& cb = ThreadInitCallback());

			EventLoop* getNextLoop();
			std::vector<EventLoop*> getAllLoops();

			// clang-format off
			void setThreadNum(int numThreads){ numThreads_ = numThreads; }
			bool start() const{ return started_; }
			const std::string name() const{ return name_; }
			// clang-format on
		private:
			EventLoop* baseLoop_;
			std::string name_;
			bool started_;
			int numThreads_;
			int next_;
			std::vector<std::unique_ptr<EventLoopThread>> threads_;
			std::vector<EventLoop*> loops_;
		};
	}// namespace net

}// namespace cmuduo