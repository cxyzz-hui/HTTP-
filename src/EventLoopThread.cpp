#include "EventLoopThread.hpp"
#include "EventLoop.hpp"
#include <thread>

EventLoopThread::EventLoopThread() : loop_(nullptr) {}

EventLoopThread::~EventLoopThread()
{
    //回收资源
    if(loop_)
    {
        loop_->quit();
        if(thread_.joinable())
        {
            thread_.join();
        }
    }
}

EventLoop* EventLoopThread::start()
{
    //绑定EventLoop对象
    //创建子线程
    thread_ = std::thread([this](){threadFunctor();});
    {
        std::unique_lock<std::mutex>lock(mutex_);
        while(loop_ == nullptr)
        {
            //防止虚假唤醒用while
            cond_.wait(lock);
        }
    }

    return loop_;     
}

void EventLoopThread::threadFunctor()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex>lock(mutex_);
        loop_ = &loop;
        //唤醒上面那个start来返回这个loop_
        cond_.notify_one();
    }

    loop.loop();
    //自动加锁和解锁
    //这里加锁防止多线程死锁同时也防止了悬空指针，very safe
    std::lock_guard<std::mutex>lock(mutex_);
    loop_ = nullptr;
}