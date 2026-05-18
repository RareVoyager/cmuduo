#pragma once

#include <cstddef>
#include <string>
#include <vector>
namespace cmuduo
{
	namespace net
	{

		class Buffer
		{
		public:
			static const size_t kCheapPrepend = 8;
			static const size_t kInitiaSize = 1024;

			explicit Buffer(size_t initiaSize = kInitiaSize)
				: buffer_(kCheapPrepend + kInitiaSize),
				  readerIndex_(kCheapPrepend),
				  writeIndex_(kCheapPrepend)
			{
			}

			// clang-format off
            // 可读区域的大小
            size_t readableBytes() { return writeIndex_ - readerIndex_; }
            // 可写入区域的大小
            size_t writableBytes() { return buffer_.size() - writeIndex_; }
            // 已经读取的大小
            size_t perpendableBytes() { return readerIndex_; }
            // 可写区域的起始位置
            const char* peek() const { return begin() + readerIndex_; }

			// clang-format on

			// 缓冲区中拿走长度为len的数据 一般是onMessage 调用 Buffer -> string
			void retrieve(size_t len)
			{
				if (len < readableBytes())
				{
					readerIndex_ += len;
				}
				else
				{
					retrieveAll();
				}
			}

			void retrieveAll()
			{
				readerIndex_ = kCheapPrepend;
				writeIndex_ = kCheapPrepend;
			}

			std::string retrieveAllAsString()
			{
				return retrieveAsString(readableBytes());
			}

			std::string retrieveAsString(size_t len)
			{
				std::string result(peek(), len);
				retrieve(len);
				return result;
			}

			void ensureWriteableBytes(size_t len)
			{
				if (writableBytes() < len)
				{
					makeSpace(len);
				}
			}

			void append(const char* data, size_t len)
			{
				ensureWriteableBytes(len);
				std::copy(data, data + len, beginWrite());
				writeIndex_ += len;
			}

			// 从fd 上读取数据
			ssize_t readFd(int fd, int* saveErrno);

			ssize_t writeFd(int fd, int* saveErrno);

		private:
			// clang-format off
            char* begin() { return &*buffer_.begin();}
            const char* begin() const { return &*buffer_.begin();}
            char* beginWrite(){ return begin() + writeIndex_; }
            const char* beginWrite() const { return begin() + writeIndex_; }
			// clang-format on

			void makeSpace(size_t len)
			{
				if (writableBytes() + perpendableBytes() < len + kCheapPrepend)
				{
					buffer_.resize(len + writeIndex_);
				}
				else
				{
					size_t readable = readableBytes();
					std::copy(begin() + readerIndex_, begin() + writeIndex_, begin() + kCheapPrepend);
					readerIndex_ = kCheapPrepend;
					writeIndex_ = kCheapPrepend + readable;
				}
			}


		private:
			std::vector<char> buffer_;
			size_t readerIndex_;
			size_t writeIndex_;
		};

	}// namespace net

}// namespace cmuduo
