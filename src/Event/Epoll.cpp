#include "Epoll.hpp"
#include "Channel.hpp"
#include "../Util/util.hpp"
#include <string.h>

const int SIZE = 1024;

Epoll::Epoll() : epfd(epoll_create(1)) , events(new epoll_event[SIZE])
{
    //先进行数据初始化
    perror_if(epfd == -1 , "create epfd error in 11 line at Epoll.cpp");
    memset(events , 0 , SIZE * sizeof(epoll_event));
}

Epoll::~Epoll()
{
    //释放下内存
    if(epfd != -1)
    {
        epfd = -1;
    }
    delete[] events;
}

//更新数据
void Epoll::updateChannel(Channel *ch)
{
    int fd = ch->FD();
    struct epoll_event ev; 

    //获取ch里的所有数据
    ev.data.ptr = ch;
    ev.events = ch->Event();            //获取要监视的事件
    
    if(ch->isInEpoll())
    {
        if(ch->isNoneEvent())
        {
            del(ch);
        }
        else
        {
            //如果以及在epoll上那就修改一下事件的配置
            int ret = epoll_ctl(epfd , EPOLL_CTL_MOD , fd , &ev);
            perror_if(ret == -1 , "epoll_ctl error in 38 line at Epoll.cpp");
        }
    }
    else
    {
        //没有的haul就加上
        int ret = epoll_ctl(epfd , EPOLL_CTL_ADD , fd , &ev);
        perror_if(ret == -1 , "epoll_ctl error in 45 line at Epoll.cpp");

        //设置为已经到epoll上了
        ch->setInEpoll(true);
    }

}

void Epoll::del(Channel *ch)
{
    //增加安全性
    if(ch->isInEpoll())
    {
        int ret = epoll_ctl(epfd , EPOLL_CTL_DEL , ch->FD() , nullptr);
        perror_if(ret == -1 , "epoll_ctl error in 56 line at Epoll.cpp");

        //更新一下是否在Epoll上
        ch->setInEpoll(false);
    }
}

void Epoll::Epoll_wait(std::vector<Channel*>&active , int timeout)
{
    //看看当前一个有几个套接字有反应
    int nums = ::epoll_wait(epfd , events , SIZE , timeout);
    perror_if(nums == -1, "epoll_wait in 77 line at Epoll.cpp");
    for(int i = 0; i < nums; i++)
    {
        //将void *类型强转为Channel类型
        Channel *ch = static_cast<Channel*>(events[i].data.ptr);

        //设置实际发生的事件
        ch->setRevents(events[i].events);

        //尾差后续再处理
        active.emplace_back(ch);
    }
}
