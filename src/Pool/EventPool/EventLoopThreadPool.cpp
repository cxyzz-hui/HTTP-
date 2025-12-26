#include "EventLoopThread.hpp"
#include "EventLoopThreadPool.hpp"
#include "../../Event/EventLoop.hpp"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* basePool) 
: baseLoop_(basePool)
, started_(false)
, numThreads_(0)
, next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{
    //释放内存
}

void EventLoopThreadPool::start()
{
    started_ = true;
    for(int i = 0; i < numThreads_; i++)
    {
        //创建循环并进行管理
        auto thread = new EventLoopThread;
        threads_.push_back(std::unique_ptr<EventLoopThread>(thread));

        //开始循环后放入里面
        loops_.push_back(thread->start());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    //循环利用
    //这里用这个其实只是防止一下空指针
    EventLoop *loop = baseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_ %= numThreads_;
    }
    return loop;
}