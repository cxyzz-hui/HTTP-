#pragma
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "InetAddr.hpp"

//主要实现新建服务器socket、绑定IP地址、监听、接受客户端连接等任务。

class Socket
{
public:    
    //用this指针就可以避免重复命名的情况了
    //默认构造函数，方便创建数组和向量啥的
    Socket();
    ~Socket();
    
    Socket(int fd);

    void bind(const InetAddr  &serv_addr);
    int accept(InetAddr *addr);
    void listen();

    void setNonblock();
    int fd() {return sockfd_;}

private:

    int sockfd_;
};


#endif