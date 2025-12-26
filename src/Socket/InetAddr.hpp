#pragma
#ifndef INETADDR_HPP
#define INETADDR_HPP


//主要是关于IP地址相关的使用

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class InetAddr
{
private:
    struct sockaddr_in addr_;
public:
    InetAddr();
    InetAddr(unsigned short port , const char* ip=nullptr);

    //防止隐形转化
    explicit InetAddr(const struct sockaddr_in& addr)
		: addr_(addr)
	{ }
    //返回的是一个常量引用
    //设计的十分巧妙
    const struct sockaddr_in* getSockAddr() const{return &addr_;};
    void setAddr(const struct sockaddr_in& addr) {addr_ = addr;}

    //尾部的const表示这个函数不会修改类里面的成员变量
    std::string toIp()const;        //只获得地址而不获取端口号
    std::string toIpPort() const;

    unsigned short toPort()const;

};



#endif