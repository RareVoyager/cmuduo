#pragma once

#include <functional>
#include <memory>

#include <include/buffer.h>
#include <include/timestamp.h>

namespace cmuduo
{
	namespace net
	{
		class TcpConnection;

		using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

		using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
		using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
		using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
		using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, base::TimeStamp)>;
		using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
	}// namespace net
}// namespace cmuduo