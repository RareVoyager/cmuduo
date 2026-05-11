/**
 * 具体IO多路复用实现
 */

#include <include/poller.h>
#include <sys/epoll.h>
#include <vector>

namespace cmuduo
{
	namespace net
	{
		class EpollPoller : public Poller
		{
		public:
			EpollPoller(EventLoop* loop);
			~EpollPoller() override;

			// 实现虚方法
			cmuduo::base::TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override;
			void updateChannel(Channel* channel) override;
			void removeChannel(Channel* channel) override;

		private:
			static const int kInitEventListSize = 16;

			// 填写活跃连接
			void fillActiveChannel(int numEvents, ChannelList* activeChannels);
			// 更新channel通道
			void update(int operation, Channel* channel);
			using EventList = std::vector<epoll_event>;
			int epollfd_;
			EventList events_;
		};

	}// namespace net

}// namespace cmuduo