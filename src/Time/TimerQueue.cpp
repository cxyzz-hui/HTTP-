#include "TimerQueue.hpp"
#include "../Event/EventLoop.hpp"
#include <sys/timerfd.h>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <cstring>
#include <string>

//创建timer文件描述符用于事件循环
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC , TFD_NONBLOCK | TFD_CLOEXEC);
    //创造单调非堵塞的定时器
    if(timerfd < 0)
    {
        std::cout << "Failed in timerfd_create" << std::endl;
    }
    return timerfd;
}

/*
    timespec结构体如下
    struct timespec 
    {
        time_t tv_sec;        秒 
        long   tv_nsec;       纳秒 
    };
*/

struct timespec howMunchTimerFromNow(Timestamp when)
{
    //计算时间差
    int64_t microSeconds = when.microSecondSinceEpoch() - Timestamp::now().microSecondSinceEpoch();

    //保护最小时间差来提高性能
    if(microSeconds < 100)
    {
        microSeconds = 100;
    }

    //接下来将我们的数据强转为timespec类型的成员
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microSeconds / Timestamp::KMicroSecondPerSecond);           //获取秒
    ts.tv_nsec = static_cast<long>((microSeconds % Timestamp::KMicroSecondPerSecond) * 1000);   //获取毫秒

    return ts;
}

/*
    struct itimerspec 
    {
        struct timespec it_interval;  // 定时器的重复间隔
        struct timespec it_value;     // 定时器的首次到期时间
    };
*/

void resetTimerfd(int timerfd , Timestamp expiration)
{
    //设置timerfd的触发事件
    struct itimerspec new_value;
    memset(&new_value , 0 , sizeof(new_value));

    new_value.it_value = howMunchTimerFromNow(expiration);          //获取expiration与当前时间的间隔

    //重新设置定时器的配置
    //这里不设置重复间隔那个参数的原因是:重复间隔的那个功能归读事件管理，如果有读事件的话，那么就重复使用，而不是只是盯着一个时间来重复使用
    int ret = ::timerfd_settime(timerfd , 0 , &new_value , nullptr);    //启动定时器
    if(ret != 0)
    {
        std::cout << "timerfd_settime error" << std::endl;
    }
}

//读取定时器的到时的次数
void readTimerfd(int timerfd , Timestamp now)
{
    uint64_t howmany;
    auto n = ::read(timerfd , &howmany , sizeof(howmany));
    printf("TimerQueue::handleRead() at %s\n" , now.toString().c_str());

    if(n != sizeof(howmany))
    {
        printf("TimerQueue::handleRead() reads %ld bytes instead of 8" , n);
    }
}

TimerQueue::TimerQueue(EventLoop* loop)
    :loop_(loop)
    ,timerfd_(createTimerfd())
    ,timerfd_channel_(loop , timerfd_)
{
    //设置和注册回调函数
    timerfd_channel_.SetReadCallback([this](){handRead();});
    timerfd_channel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    //注销事件并且在epoll在取消这个Channel类
    timerfd_channel_.disableAll();
    timerfd_channel_.remove();

    close(timerfd_);
    for(const auto& timer : timers_)
    {
        delete timer.second;
    }
}

//添加计时器，返回的是计时器的唯一标识
int64_t TimerQueue::addTime(Timer::TimerCallback cb , Timestamp when , double interval)
{
    auto timer = new Timer(std::move(cb) , when , interval);
    loop_->runInLoop([this , &timer](){addTimerInLoop(timer);});
    return timer->sequence();
}

void TimerQueue::cancel(int64_t timerId)
{
    loop_->runInLoop([this , timerId](){cancelInLoop(timerId);});
}

//执行定时器的任务
void TimerQueue::handRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_ , now);            //调用read防止一直执行handleRead函数

    auto expired = getExpired(now);         //获取所有的超时容器
    callingExpiredTimers_ = true;

    //处理到期定时器
    cancelingTimer_.clear();
    for(const auto& it : expired)
    {
        it.second->run();
    }

    callingExpiredTimers_ = false;
    //到期的定时器处理完后,需要进行重置最早的到期时间
    reset(expired , now);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    //看看这个玩意超时了没有超时了记得重新设置一下参数

    bool earliestChanged = insert(timer);  
    if(earliestChanged)
    {
        resetTimerfd(timerfd_ , timer->expiration());
    }
}

void TimerQueue::cancelInLoop(int64_t timerId)
{

    auto it = active_timer_.find(timerId);
    if(it != active_timer_.end())
    {
        timers_.erase(Entry(it->second->expiration() , it->second));
        delete it->second;
        active_timer_.erase(it);
    }
    else if(callingExpiredTimers_)
    {
        //如果定时器正在执行的话，那么它也不在active_timer里面
        cancelingTimer_.emplace(timerId , it->second);
    }

    //它们俩是同步关系
    assert(active_timer_.size() == timers_.size());
}

//得到那些过期的计时器
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    //这里sentry的第二个强转参数的实际意义在于创建一个上届，使进行比较的地址都比这个小，因为在时间相同的时候我们此时是不关心地址的
    Entry sentry(now , reinterpret_cast<Timer*>(UINTPTR_MAX));
    //二分查找第一个>=now时间戳的计时器
    auto end = timers_.lower_bound(sentry);

    std::vector<Entry> expired(timers_.begin() , end);
    timers_.erase(timers_.begin() , end);
    //去除active中相应过期的计时器
    for(const auto& it : expired)
    {
        active_timer_.erase(it.second->sequence());
    }

    assert(active_timer_.size() == timers_.size());
    return expired;
}

bool TimerQueue::insert(Timer* timer)
{
    //这里要修改二者的空间，所以要加上截断
    assert(active_timer_.size() == timers_.size());

    bool earlistChanged = false;

    auto when = timer->expiration();        //获取它的到时 时间
    auto it = timers_.begin();              //获取最小的那个元素

    if(it == timers_.end() || when < it->first)
    {
        earlistChanged = true;
        //若容器中没有元素 或者 要增添的定时器的到期时间 < timers_容器中的最早的到期时间(10点 < 11点)，那么我们就要重置这个最小到时时间
    }

    //在这两个容器都添加这个定时器
    timers_.emplace(Entry(when , timer));
    active_timer_.emplace(timer->sequence() , timer);

    assert(active_timer_.size() == timers_.size());

    return earlistChanged;
}

/*
    这个函数的功能是处理那些重复的计时器，让那么可以被使用的计时器继续使用，不能使用的就删去
    同时设置好下一次的唤醒时间
*/

void TimerQueue::reset(const std::vector<Entry>& expired , Timestamp now)
{   
    for(const auto& it : expired)
    {
        //如果这个计时器是重复的并且没有在删除，那么就把它重新插入
        if(it.second->repeat() && cancelingTimer_.find(it.second->sequence()) == cancelingTimer_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    } 

    if(!timers_.empty())
    {
        //获取下一个超时的事件
        Timestamp nextExpired = timers_.begin()->second->expiration();
        if(nextExpired.valid())
        {
            //如果时间有效那么就设置这个为下一个超时时限
            resetTimerfd(timerfd_ , nextExpired);
        }
    }   

}