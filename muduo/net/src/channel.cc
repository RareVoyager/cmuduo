#include <sys/epoll.h>

#include <include/channel.h>
#include <include/log.h>
namespace cmuduo
{
    namespace net
    {
        const int Channel::KNoneEvent = 0;
        const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
        const int Channel::KWriteEvent = EPOLLOUT;

        Channel::Channel(EventLoop *loop, int fd)
            : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
        {}
        Channel::~Channel() {}

        void Channel::tie(const std::shared_ptr<void> &obj)
        {
            tie_ = obj;
            tied_ = true;
        }

        void Channel::update()
        {
            /**
             * add code
             */
        }

        void Channel::remove()
        {
            /**
             * add code
             */
        }

        void Channel::handleEvent(base::TimeStamp receiveTime)
        {
            if (tied_) {
                std::shared_ptr<void> guard = tie_.lock();
                if (guard) {
                    handleEventWithGuard(receiveTime);
                }
            }
            else {
                handleEventWithGuard(receiveTime);
            }
        }

        void Channel::handleEventWithGuard(base::TimeStamp receiveTime)
        {
            // EPOLLHUP: 连接挂起，对端关闭、连接断开、fd 异常时可能出现
            // EPOLLIN:  fd 上有数据可读，包括 socket 收到数据、listenfd 有新连接到来
            // EPOLLERR: fd 发生错误，例如连接异常、socket 错误；通常不需要主动注册，发生时 epoll 也会返回

            // EPOLLOUT: fd 可写，通常表示 socket 发送缓冲区有空间，可以继续写数据
            // EPOLLPRI: fd 上有紧急数据可读，例如 TCP 带外数据，普通网络库较少使用

            LOG_INFO("channel handleEvent revents%d", revents_);
            if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
                if (closeCallback_) {
                    closeCallback_();
                }
            }

            if ((revents_ & EPOLLERR)) {
                if (errorCallback_) {
                    errorCallback_();
                }
            }

            if (revents_ & (EPOLLIN | EPOLLPRI)) {
                if (readCallback_) {
                    readCallback_(receiveTime);
                }
            }

            if ((revents_ & EPOLLOUT)) {
                if (writeCallback_) {
                    writeCallback_();
                }
            }
        }

    } // namespace net

} // namespace cmuduo
