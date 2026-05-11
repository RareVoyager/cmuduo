#include <include/log.h>
#include <include/inetaddress.h>

int main(){
    cmuduo::base::InetAddress address(9527,"171.22.14.19");
    LOG_INFO(address.toIp().c_str());
    LOG_INFO(address.toIpPort().c_str());
    uint16_t port_i = address.toPort();
    std::string port = std::to_string(port_i);
    LOG_INFO(port.c_str());

    return 0;
}