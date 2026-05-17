#include <arpa/inet.h>
#include <cstring>
#include <include/inetaddress.h>
#include <strings.h>

namespace cmuduo
{
	namespace base
	{
		InetAddress::InetAddress(uint16_t port, std::string ip)
		{
			bzero(&addr_, sizeof addr_);
			// ipv4 族
			addr_.sin_family = AF_INET;
			// 本地字节序转网络字节序
			addr_.sin_port = htons(port);
			addr_.sin_addr.s_addr = ::inet_addr(ip.c_str());
		}
		InetAddress::InetAddress(sockaddr_in addr) : addr_(addr)
		{
		}

		std::string InetAddress::toIp()
		{
			char buf[64] = {0};
			::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
			return buf;
		}

		uint16_t InetAddress::toPort() const
		{
			return ntohs(addr_.sin_port);
		}

		std::string InetAddress::toIpPort() const
		{
			char buf[64] = {0};
			::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
			size_t end = strlen(buf);
			sprintf(buf + end, ":%u", toPort());
			return buf;
		}
	}// namespace base
}// namespace cmuduo
