#include <include/eventloop.h>
#include <include/eventloopthread.h>
namespace cmuduo
{
	namespace net
	{
		EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
			: loop_(nullptr),
			  exiting_(false),
			  thread_(std::bind(&EventLoopThread::threadFunc, this), name),
			  mutex_(),
			  cond_(),
			  callback_(std::move(cb))
		{
		}

		EventLoopThread::~EventLoopThread()
		{
			exiting_ = false;
			if (loop_)
			{
				loop_->quit();
				thread_.join();
			}
		}

		EventLoop* EventLoopThread::startLoop()
		{
			thread_.start();

			// 线程间的通信
			EventLoop* loop = nullptr;
			{

				std::unique_lock<std::mutex> lock(mutex_);
				while (loop == nullptr)
				{
					cond_.wait(lock);
				}
				loop = loop_;
			}
			return loop;
		}


		// 以下方法在新线程里面执行
		void EventLoopThread::threadFunc()
		{
			EventLoop loop;

			if (callback_)
			{
				callback_(&loop);
			}

			{
				std::unique_lock<std::mutex>(mutex_);
				loop_ = &loop;
				cond_.notify_one();
			}

			loop.loop();
			// 此时 底层服务器要关闭，不进行事件循环了。
			std::unique_lock<std::mutex>(mutex_);
			loop_ = nullptr;
		}
	}// namespace net


}// namespace cmuduo