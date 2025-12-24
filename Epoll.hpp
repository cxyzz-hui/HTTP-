#pragma
#ifndef EPOLL_HPP
#define EPOLL_HPP

//创建epoll实例，添加/删除文件描述符到epoll上，检测文件描述符是否就绪等。
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "Channel.hpp"

class Channel;

class Epoll
{
private:
    int epfd;
    struct epoll_event* events;
public:

    int epollFd()const {return epfd;};
    //将sockfd以event事件加入op的epoll里面
    //void update(int sockfd , int evenet , int op);
    void updateChannel(Channel* ch);
	void del(Channel* ch);
    
    //void Epoll_wait(std::vector<epoll_event>&activeEvents , int timeout = 10);
    void Epoll_wait(std::vector<Channel*>& active, int timeout = 10);
    Epoll();
    ~Epoll();
};


#endif

