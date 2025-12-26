#include "InetAddr.hpp"
#include <string.h>

InetAddr::InetAddr()
{
    memset(&addr_ , 0 , sizeof(addr_));
}

InetAddr::InetAddr(unsigned short port , const char *ip)
{
    //填充信息结构体的基本信息
    memset(&addr_ , 0 , sizeof(addr_));

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);

    if(ip == nullptr)
    {
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        /*
            inet_pton函数
            功能：将字符串转化为二进制的网络字节序
            参数：IP协议 ， 要转化的地址处，目标地址处
        */
        inet_pton(AF_INET, ip, &addr_.sin_addr.s_addr);
    }
}

std::string InetAddr::toIp() const
{
    char ip[64] = {0};
    
    //功能和上面那个函数相反
    inet_ntop(AF_INET , &addr_.sin_addr.s_addr , ip , sizeof(ip));
    return ip; 
}

std::string InetAddr::toIpPort() const
{
    char buf[128] = "";
    sprintf(buf , "%s:%d" , toIp().c_str() , toPort());
    return buf;
}

unsigned short InetAddr::toPort() const
{
    return ntohs(addr_.sin_port);
}