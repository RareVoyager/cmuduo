/**
 * 封装地址结构
 */

#pragma once

#include <cstdint>
#include <string>
#include <netinet/in.h>

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

        private:
            sockaddr_in addr_;
        };

    } // namespace base

} // namespace cmuduo
