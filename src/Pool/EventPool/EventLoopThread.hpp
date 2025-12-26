#pragma
#ifndef EVENTLOOPTHREAD_HPP
#define EVENTLOOPTHREAD_HPP

#include <mutex>
#include <condition_variable>
#include <thread>

//事件循环类:从Reactor
class EventLoop;
class EventLoopThread
{
public:
    EventLoopThread(/* args */);
    ~EventLoopThread();

    EventLoop* start(); 
    //启动IO线程函数中的loop循环, 返回IO线程中创建的EventLoop对象地址(栈空间

private:
    void threadFunctor();
    //IO线程函数

    EventLoop* loop_;       
    //绑定EventLoop对象的指针
    std::thread thread_;

    //线程同步的工具
    std::mutex mutex_;
    std::condition_variable cond_;
};



#endif