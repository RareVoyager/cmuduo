

#include <cstring>
#include <errno.h>
#include <include/epollpoller.h>
#include <include/log.h>
#include <unistd.h>


namespace cmuduo
{
	namespace net
	{
		// channel 未添加到poller中
		const int kNew = -1;
		// channel 已添加到poller中
		const int kAdded = 1;
		// channel 从poller中删除
		const int kDeleted = 2;

		EpollPoller::EpollPoller(EventLoop* loop)
			: Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
		{
			if (epollfd_ < 0)
			{
				LOG_FATAL("create epoll fd error %d\n", errno);
			}
		}

		EpollPoller::~EpollPoller()
		{
			::close(epollfd_);
		}

		cmuduo::base::TimeStamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
		{
			int csize = channels_.size();
			LOG_INFO("func=>%s, fd total count = %d", __FUNCTION__, csize);

			int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), 10);
			int saveErrno = errno;
			cmuduo::base::TimeStamp now(cmuduo::base::TimeStamp::now());
			if (numEvents > 0)
			{
				LOG_INFO("%d events happend\n", numEvents);
				fillActiveChannel(numEvents, activeChannels);
				if (numEvents == csize)
				{
					channels_.reserve(csize + csize / 2);
				}
			}
			else if (numEvents == 0)
			{
				LOG_INFO("%s timeout!\n", __FUNCTION__);
			}
			else
			{
				if (saveErrno != EINTR)
				{
					errno = saveErrno;
					LOG_ERROR("EPollPoller::poll() error");
				}
			}
			return now;
		}

		// channel 的更新操作。
		void EpollPoller::updateChannel(Channel* channel)
		{
			// 先获取index
			int index = channel->index();
			LOG_INFO("func => %s fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);
			if (index == kNew || index == kDeleted)
			{
				if (index == kNew)
				{
					int fd = channel->fd();
					channels_[fd] = channel;
				}
				channel->setIndex(kAdded);
				update(EPOLL_CTL_ADD, channel);
			}
			else
			{
				// 已经在poller注册过
				// 查看channel 是否还有感兴趣的事件。如果没有就删除，有的话就修改
				if (channel->isNoneEvent())
				{
					channel->setIndex(kNew);
					update(EPOLL_CTL_DEL, channel);
				}
				else
				{
					update(EPOLL_CTL_MOD, channel);
				}
			}
		}
		void EpollPoller::removeChannel(Channel* channel)
		{
			LOG_INFO("func => %s fd=%d events=%d\n", __FUNCTION__, channel->fd(), channel->events());
			channels_.erase(channel->fd());
			if (channel->index() == kAdded)
			{
				update(EPOLL_CTL_DEL, channel);
			}
			channel->setIndex(kNew);
		}
		void EpollPoller::fillActiveChannel(int numEvents, ChannelList* activeChannels)
		{
			for (int i = 0; i < numEvents; i++)
			{
				Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
				channel->setRevents(events_[i].events);
				activeChannels->emplace_back(channel);
			}
		}
		void EpollPoller::update(int operation, Channel* channel)
		{
			epoll_event event;
			memset(&event, 0, sizeof event);
			// 设置event感兴趣的事件
			event.events = channel->events();
			event.data.ptr = channel;

			if (::epoll_ctl(epollfd_, operation, channel->fd(), &event) < 0)
			{
				if (operation == EPOLL_CTL_DEL)
				{
					LOG_ERROR("epoll_ctl delete error\n");
				}
				else
				{
					LOG_FATAL("epoll_ctl add/mod error\n");
				}
			}
		}
	}// namespace net


}// namespace cmuduo
