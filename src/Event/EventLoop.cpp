#include "../Time/TimerQueue.hpp"
#include "EventLoop.hpp"
#include "Channel.hpp"
#include "Epoll.hpp"
#include <thread>
#include <signal.h>

int createEventfd()
{
    int enfd = ::eventfd(0 , EFD_NONBLOCK | EFD_CLOEXEC);
    if(enfd < 0)
    {
        printf("eventfd error:%d\n" , errno);
    }
    return enfd;
}

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE , SIG_IGN);
    }
};

IgnoreSigPipe initObj;

EventLoop::EventLoop() 
: threadId_(std::this_thread::get_id())
, quit_(false)
, callingPendingFunctors_(false)
, ep_(std::make_unique<Epoll>())
, wakeupfd_(createEventfd())
, wakeupChannel(std::make_unique<Channel>(this , wakeupfd_))
{
    //注册回调函数
    /*
    回调函数执行过程:
        其他线程调用wakeup() → 向wakeupfd_写入数据 
        → 内核标记wakeupfd_可读 → epoll_wait返回wakeupChannel_为活跃
        → EventLoop调用wakeupChannel_->handleEvent() 
        → 执行预先设置的读回调 → 调用handleRead()
    */
    wakeupChannel->SetReadCallback([this](){handleRead();});
    //监听wakeupfd_的读事件
    wakeupChannel->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel->disableAll();
    wakeupChannel->remove();
    ::close(wakeupfd_);
}

void EventLoop::loop()
{

    quit_ = false;
    //先清空先前的活跃事件，再接受现阶段的活跃事件
    while(!quit_)
    {
        activeChannel.clear();

        //获取本线程活跃事件的数量
        ep_->Epoll_wait(activeChannel , 10000);
        for(auto& active : activeChannel)
        {
            active->handleEvent();
        }
        //处理其他线程传来的函数
        doPendingFunctor();
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    ep_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    ep_->del(channel);
}

void EventLoop::quit()
{
    quit_ = true;
}

void EventLoop::runInLoop(Functor cb)
{
    //printf("Call EventLoop::runInLoop when not in the IO thread\n");
    if(isInLoopThread())
    {
        //printf("EventLoop::runInLoop() In this IO thread , directly call the callback function\n");
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }

}

void EventLoop::queueInLoop(Functor cb)
{
    //printf("Call EventLoop::queueInLoop when not in the IO thread");
    {
        //插入要执行的回调函数
        std::unique_lock<std::mutex>lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }

}

// 用来唤醒loop所在线程  向wakeupfd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup()
{
    //printf("need to weak up IO thread , call EventLoop::wakeup\n");

    uint64_t one = 1;
    ssize_t n = ::write(wakeupfd_ , &one , sizeof(one));
    if(n != sizeof(one))
    {
        printf("EventLoop wakeup write %lu bytes instead of 8\n" , n);
    }
}

void EventLoop::handleRead()
{
    //printf("need to weak up IO thread , call EventLoop::wakeup\n");

    uint64_t one = 1;
    ssize_t n = ::read(wakeupfd_ , &one , sizeof(one));
    if(n != sizeof(one))
    {
        printf("EventLoop::handleRead() reads %lu bytes \n", n);
    }
}

//执行回调函数
void EventLoop::doPendingFunctor()
{
    std::vector<Functor>functors;
    callingPendingFunctors_ = true;
    // 把functors转移到局部的functors，这样在执行回调时不用加锁。不影响mainloop注册回调,还可以节省性能
    {
        std::unique_lock<std::mutex>lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(auto &functor : functors)
    {
        //printf("Execute the callback function functor() in EventLoop:: doPendingFunctors()\n");
        //执行当前循环需要执行的函数
        functor();
    }

    callingPendingFunctors_ = false;
}

//在特定时间添加计时器相关操作
int64_t EventLoop::runAt(Timestamp time , TimerCallback cb)
{
    return TimeQueue_->addTime(std::move(cb), time, 0.0);
}

//在给定时间之后添加计算器相关操作
int64_t EventLoop::runAfter(double delay_seconds , TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now() , delay_seconds));
    return  runAt(time , std::move(cb));
}

//每隔一个时间间隔添加计时器(添加重复计时器)
int64_t EventLoop::runEntry(double interval_seconds , TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now() , interval_seconds));
    return TimeQueue_->addTime(std::move(cb) , time , interval_seconds);
}

//取消计时器
void EventLoop::cancel(int64_t TimerId)
{
    return TimeQueue_->cancel(TimerId);
}