
#include <include/eventloop.h>
#include <include/eventloopthread.h>
#include <include/eventloopthreadpool.h>

namespace cmuduo
{
	namespace net
	{
		EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg)
			: baseLoop_(baseloop),
			  name_(nameArg),
			  started_(false),
			  numThreads_(0),
			  next_(0)
		{
		}

		EventLoopThreadPool::~EventLoopThreadPool()
		{
		}

		void EventLoopThreadPool::start(const ThreadInitCallback& cb)
		{
			started_ = true;
			for (int i = 0; i < numThreads_; i++)
			{
				char buf[name_.size() + 32];
				snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
				EventLoopThread* t = new EventLoopThread(cb, buf);
				threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
				loops_.emplace_back(t->startLoop());
			}

			if (numThreads_ == 0 && cb)
			{
				cb(baseLoop_);
			}
		}

		EventLoop* EventLoopThreadPool::getNextLoop()
		{
			EventLoop* loop = baseLoop_;

			if (!threads_.empty())
			{
				loop = loops_[next_++];
				if (next_ > loops_.size())
				{
					next_ = 0;
				}
			}
			return loop;
		}
		std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
		{
			if (numThreads_ == 0)
			{
				return std::vector<EventLoop*>(1, baseLoop_);
			}
			else
			{
				return loops_;
			}
		}// namespace net

	}// namespace net


}// namespace cmuduo
