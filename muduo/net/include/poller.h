/**
 * 抽象了IO多路复用
 */
#pragma once
#include <unordered_map>
#include <vector>

#include <include/channel.h>
#include <include/noncopyable.h>
#include <include/timestamp.h>
namespace cmuduo
{
	namespace net
	{
		class EventLoop;
		class Poller : base::noncopyable
		{
		public:
			using ChannelList = std::vector<Channel*>;
			Poller(EventLoop* loop);
			virtual ~Poller() = default;

			virtual cmuduo::base::TimeStamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
			virtual void updateChannel(Channel* channel) = 0;
			virtual void removeChannel(Channel* channel) = 0;

			bool hasChannel(Channel* channel);

			// 通过该接口获取默认的IO多路复用的具体实现
			static Poller* newDefaultPoller(EventLoop* loop);

		protected:
			using ChannelMap = std::unordered_map<int, Channel*>;
			ChannelMap channels_;

		private:
			EventLoop* ownerLoop_;
		};
	}// namespace net
}// namespace cmuduo