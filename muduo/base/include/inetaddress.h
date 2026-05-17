/**
 * 封装地址结构
 */

#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>

namespace cmuduo
{
	namespace base
	{
		class InetAddress
		{
		public:
			explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
			explicit InetAddress(sockaddr_in addr);

			std::string toIp();
			uint16_t toPort();
			std::string toIpPort();

			const sockaddr_in* getSockAddr() const
			{ return &addr_; }

			void setSockAddr(const sockaddr_in addr)
			{addr_ = addr;}

		private:
			sockaddr_in addr_;
		};

	}// namespace base

}// namespace cmuduo
