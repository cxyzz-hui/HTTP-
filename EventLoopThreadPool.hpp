#pragma
#ifndef EVENTLOOPTHREADPOOL_HPP
#define EVENTLOOPTHREADPOOL_HPP

#include <vector>
#include <functional>
#include <memory>

class EventLoop;
class EventLoopThread;

//管理从Reactor线程的线程池
class EventLoopThreadPool
{
private:
    std::vector<EventLoop*>loops_;
    //向的是EventLoopThread线程函数创建的EventLoop对象
    std::vector<std::unique_ptr<EventLoopThread>>threads_;
    //IO线程列表:使用指针管理，节省空间

    //这个baseLoop是主Reactor线程
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;  
    //新连接到来，所选择的EventLoopThread下标
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();

    void start();
    EventLoop* getNextLoop();

    //工具函数
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    bool started()const  {return started_;}
};


#endif