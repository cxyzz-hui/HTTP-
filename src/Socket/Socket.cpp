#include "../Util/util.hpp"
#include "Socket.hpp"
#include "InetAddr.hpp"
#include <fcntl.h>
#include <unistd.h>

Socket::Socket() : sockfd_(socket(AF_INET , SOCK_STREAM , 0))
{
    perror_if(sockfd_ == -1 , "socket error in 9 line at Socket.cpp");
}

Socket::Socket(int fd) : sockfd_(fd)
{
    perror_if(sockfd_ == -1 , "socket error in 14 line at Socket.cpp");
}

void Socket::bind(const InetAddr &serv_addr)
{
    //绑定结构体，这里用一个类里面的变量来代替
    //记得获得引用
    int ret = ::bind(sockfd_ , (struct sockaddr*)serv_addr.getSockAddr() , sizeof(sockaddr_in));
    perror_if(ret == -1 , "bind error in 22 line at Socket.cpp");
}

//绑定客户端结构体并且获取客户端套接字
int Socket::accept(InetAddr* addr)
{
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int cfd = ::accept4(sockfd_ , (struct sockaddr *)&client_addr , &len , SOCK_NONBLOCK | SOCK_CLOEXEC);
     
    perror_if(cfd == -1 , "accept error in 32 line at Socket.cpp");

    addr->setAddr(client_addr);
    printf("new client fd %d ip:%s ,port:%d connected..\n", cfd, addr->toIp().c_str(), addr->toPort());

    return cfd;
}

void Socket::listen()
{
    //开始监听
    int ret = ::listen(sockfd_ , 128);
    perror_if(ret == -1 , "listen error in 44 line at Socket.cpp");
}

//为文件套接字添加一个非堵塞的状态
void Socket::setNonblock()
{
    int flag = fcntl(sockfd_ , F_GETFL);            //获取套接字描述符的状态
    flag |= O_NONBLOCK;                             //添加一个非堵塞状态
    fcntl(sockfd_ , F_SETFL , flag);                //设置新的状态
}

Socket::~Socket()
{
    if(sockfd_ != -1)
    {
        close(sockfd_);
        sockfd_ = -1;
    }
}