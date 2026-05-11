/**
     * 通道
     */
#pragma once
#include <functional>
#include <include/timestamp.h>
#include <memory>
namespace cmuduo
{
	namespace net
	{
		class EventLoop;
		class Channel
		{
		public:
			using ReadEventCallback = std::function<void(base::TimeStamp)>;
			using EventCallback = std::function<void()>;

			Channel(EventLoop* loop, int fd);
			~Channel();

			// fd 得到poller通知，处理事件
			void handleEvent(base::TimeStamp receiveTime);

			void tie(const std::shared_ptr<void>&);

			void remove();

			// clang-format off
                void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
                void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
                void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
                void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

                int fd() const { return fd_; }
                int events() const { return events_; }
                void setRevents(int revt) { revents_ = revt; }
                int index(){ return index_; }
                void setIndex(int idx) { index_ = idx;}

                // 设置fd感兴趣的事件
                void enableReading(){ events_ |= KReadEvent; update(); }
                void disableReading(){ events_ &= ~KReadEvent; update(); }
                void enableWriting(){ events_ |= KWriteEvent; update();}
                void disableWriting(){ events_ &= ~KWriteEvent; update(); }
                void disableAll(){ events_ = KNoneEvent; update(); }

                // fd 当前的事件状态
                bool isNoneEvent() { return events_ == KNoneEvent; }
                bool isReading() { return events_ & KReadEvent; }
                bool isWriting() {return events_ & KWriteEvent; }

                EventLoop *ownerLoop() { return loop_;}
                // clang-format off

                

            private:
                void update();

                void handleEventWithGuard(base::TimeStamp receiveTime);

                // void handle
            private:
                static const int KNoneEvent;
                static const int KReadEvent;
                static const int KWriteEvent;

                EventLoop *loop_;
                int fd_;
                // fd 感兴趣的事件
                int events_;
                // poller 具体发生的事件
                int revents_;

                int index_;

                std::weak_ptr<void> tie_;
                bool tied_;

                ReadEventCallback readCallback_;
                EventCallback writeCallback_;
                EventCallback closeCallback_;
                EventCallback errorCallback_;
            };
        } // namespace net

    } // namespace cmuduo
