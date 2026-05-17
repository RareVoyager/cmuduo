#include <errno.h>
#include <sys/uio.h>

#include <include/buffer.h>


namespace cmuduo
{
	namespace net
	{
		// 从fd 读取数据 默认LT模式
		ssize_t net::Buffer::readFd(int fd, int* saveErrno)
		{
			// 栈上的64 空间
			char extrabuf[65536] = {0};
			iovec vec[2];

			// 剩余大小
			const size_t writable = writableBytes();
			vec[0].iov_base = begin() + writeIndex_;
			vec[0].iov_len = writable;

			vec[1].iov_base = extrabuf;
			vec[1].iov_len = sizeof(extrabuf);

			const int iovcnt = writable < sizeof(extrabuf) ? 2 : 1;
            
            // readv 先向vec[0] 写数据，再向vec[1] 写数据
			// 向非连续的缓冲区写数据
            ssize_t n = ::readv(fd, vec, iovcnt);

			if (n < 0)
			{
				*saveErrno = errno;
			}
			else if (n <= writable)
			{
                // buffer_ 够写
				writeIndex_ += n;
			}
			else
			{
				writeIndex_ = buffer_.size();
				append(extrabuf, n - writable);
			}
			return n;
		}

	}// namespace net


}// namespace cmuduo
