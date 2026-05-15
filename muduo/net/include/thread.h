#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <include/noncopyable.h>
namespace cmuduo
{
	namespace net
	{

		class Thread : base::noncopyable
		{
		public:
			using ThreadFunc = std::function<void()>;
			explicit Thread(ThreadFunc func, const std::string& name = std::string());
			~Thread();

			void start();
			void join();

			// clang-format off
			bool started() const { return started_; }
            bool joined() const { return joined_; }

            pid_t tid() const { return tid_; }
            
            const std::string name() const { return name_; }
            static int numCreated() { return numCreated_; }
			// clang-format on

		private:
			void setDefaultName();

		private:
			bool started_;
			bool joined_;
			// 只能指针来管理线程的创建
			std::shared_ptr<std::thread> thread_;
			// 线程的回调函数
			ThreadFunc func_;
			pid_t tid_;
			// 线程名
			std::string name_;

			// 静态变量, 创建的线程数
			static std::atomic_int numCreated_;
		};

	}// namespace net

}// namespace cmuduo