#pragma
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

//我们这里引入一下Channel类的作用
//虽然我们的Epoll结构体重有fd来存放文件描述符，但我们不能获得它的事件。
//这个类就是使用了Epoll结构体中的空指针void *来存放其他数据
//我们可以把一个文件描述符封装成一个类，这个类里面有更多的关于这个文件描述符的信息，我们把他叫做Channel类，用这个void*指针指向Channel类对象。


//Channel类的作用，一个Channel对应一个套接字描述符，并且代表者它的多种IO操作
#include "EventLoop.hpp"
#include <vector>
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <memory>

class EventLoop;

class Channel
{
public:
    //简化一下
	using ReadEventCallback = std::function<void()>;
	using EventCallback = std::function<void()>;
private:
    //Epoll *ep_;
    EventLoop* loop_;
    int fd_;
    int events_;            //关心的事件
    int revents_;           //实际发生的事件
    bool isInEpoll_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    //可以weak_ptr<void>可以容纳任何类型的弱指针
    std::weak_ptr<void> tie_;
    bool tied;

public:
    //Channel(Epoll * ep , int fd);
    Channel(EventLoop *loop , int fd);

    void setEvent(int events);
    int Event()const;

    void setRevents(int revents);
    int Revent()const;
    
    bool isInEpoll();
    void setInEpoll(bool in);
    int FD();

    void remove();

    //设置回调函数
    void SetReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
	void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
	void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) {errorCallback_ = std::move(cb);}
    void handleEvent();

    //注册事件
	void enableReading() { events_ |= (EPOLLIN | EPOLLPRI); update(); }	//注册可读事件
	void disableReading() { events_ &= ~(EPOLLIN | EPOLLPRI); update(); }	//注销可读事件
	void enableWriting() { events_ |= EPOLLOUT; update(); }			//注册可写事件
	void disableWriting() { events_ &= ~EPOLLOUT; update(); }		//注销可写事件
	void disableAll() {events_ = 0; update();}	//注销所有事件

    //判断此时事件的状态
    bool isNoneEvent()const {return events_ == 0;}
    bool isWrite() const { return events_ & EPOLLOUT; }
    bool isReading() const { return events_ & (EPOLLIN | EPOLLPRI); }

    void tie(const std::shared_ptr<void>&a);
private:
    void update();

    void handleEventWithGuard();
};


#endif