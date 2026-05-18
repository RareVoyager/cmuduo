
#include <sys/eventfd.h>

#include <include/eventloop.h>
#include <include/log.h>
#include <include/poller.h>

// 防止一个线程有多个EventLoop
__thread cmuduo::net::EventLoop* t_loopInThisThread = nullptr;

// IO 复用接口超时时间 10s
const int kPollTimeMs = 10000;

int createEventfd()
{
	int evefd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evefd < 0)
	{
		LOG_FATAL("create eventfd faild");
	}
	return evefd;
}

namespace cmuduo
{
	namespace net
	{
		EventLoop::EventLoop()
			: looping_(false),
			  quit_(false),
			  callingPendingFunctors_(false),
			  threadId_(CurrentThread::tid()),
			  wakeupFd_(createEventfd()),
			  poller_(Poller::newDefaultPoller(this)),
			  wakeupChannel_(new Channel(this, wakeupFd_))
		{
			// LOG_DEBUG("EventLoop created %p in thread %d\n", this, threadId_);
			if (t_loopInThisThread)
			{
				LOG_FATAL("EventLoop exists %p\n", this);
			}
			else
			{
				t_loopInThisThread = this;
			}

			LOG_INFO("EventLoop Init finished");
			wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
			wakeupChannel_->enableReading();
		}

		EventLoop::~EventLoop()
		{
			wakeupChannel_->disableAll();
			wakeupChannel_->remove();
			::close(wakeupFd_);
			t_loopInThisThread = nullptr;
		}


		void EventLoop::loop()
		{
			looping_ = true;
			quit_ = false;
			LOG_INFO("EventLoop %p start loop ", this);

			while (!quit_)
			{
				activeChannels_.clear();
				pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

				for (auto channel: activeChannels_)
				{
					channel->handleEvent(pollReturnTime_);
				}

				// 执行当前EventLoop 事件循环需要处理的回调操作
				doPendingFunctors();
				LOG_INFO("EventLoop %p stop loop ", this);
			}
		}

		void EventLoop::quit()
		{
			quit_ = true;
			// 其他线程调用
			if (!isInLoopThread())
			{
				wakeup();
			}
		}

		void EventLoop::runInLoop(Functor cb)
		{
			if (isInLoopThread())
			{
				cb();
			}
			else// 非当前线程中执行
			{
				queueInLoop(cb);
			}
		}

		void EventLoop::queueInLoop(Functor cb)
		{
			{
				std::unique_lock<std::mutex> lock(mutex_);
				pendingFunctor_.emplace_back(cb);
			}
			if (!isInLoopThread() || callingPendingFunctors_)
			{
				wakeup();
			}
		}
		void EventLoop::wakeup()
		{
			uint64_t one = 1;
			ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
			if (n != sizeof(one))
			{
				LOG_ERROR("");
			}
		}

		void EventLoop::updateChannel(Channel* channel)
		{
			poller_->updateChannel(channel);
		}

		void EventLoop::removeChannel(Channel* channel)
		{
			poller_->removeChannel(channel);
		}

		bool EventLoop::hasChannel(Channel* channel)
		{
			return poller_->hasChannel(channel);
		}

		void EventLoop::handleRead()
		{
			uint64_t one = 1;
			ssize_t n = ::read(wakeupFd_, &one, sizeof one);
			if (n != sizeof(one))
			{
				LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8", n);
			}
		}

		void EventLoop::doPendingFunctors()
		{
			std::vector<Functor> functors;
			callingPendingFunctors_ = true;

			{
				std::unique_lock<std::mutex> lock(mutex_);
				functors.swap(pendingFunctor_);
			}

			for (auto it: functors)
			{
				// 当前loop 需要执行的回调操作。
				it();
			}
			callingPendingFunctors_ = false;
		}

	}// namespace net

}// namespace cmuduo
// namespace cmuduo