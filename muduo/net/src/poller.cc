#include <include/poller.h>

namespace cmuduo
{
	namespace net
	{
		Poller::Poller(EventLoop* loop) : ownerLoop_(loop)
		{
		}

		bool Poller::hasChannel(Channel* channel)
		{
			auto it = channels_.find(channel->fd());
			return it != channels_.end() && it->second == channel;
		}


	}// namespace net
}// namespace cmuduo
