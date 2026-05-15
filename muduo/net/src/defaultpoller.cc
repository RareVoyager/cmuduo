#include <include/epollpoller.h>
#include <include/poller.h>
namespace cmuduo
{
	namespace net
	{
		Poller* Poller::newDefaultPoller(EventLoop* loop)
		{

			if (::getenv("MODUO_USE_POLL"))
			{
				return nullptr;// 返回poll 实例对象
			}
			else
			{
				return new EpollPoller(loop);// 返回epoll 实例对象
			}
		}
	}// namespace net
}// namespace cmuduo
