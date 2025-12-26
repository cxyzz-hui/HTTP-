#pragma
#ifndef TIMERQUEUE_HPP
#define TIMERQUEUE_HPP

#include <set>
#include <cstdio>
#include <utility>
#include "Timer.hpp"
#include "../Event/Channel.hpp"
#include <unordered_map>

class TimerQueue
{
private:
    using Entry = std::pair<Timestamp , Timer*>;
    using TimerList = std::set<Entry>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(int64_t timerId);

    void reset(const std::vector<Entry>& expired , Timestamp now);
    void handRead();        //执行计时器任务
    std::vector<Entry> getExpired(Timestamp now);           //获取所有超时的计时器
    bool insert(Timer* timer);      //插入计时器

    EventLoop* loop_;
    const int timerfd_;             //timefd_的文件描述符
    Channel timerfd_channel_;       //加入epoll进行事件循环
    TimerList timers_;              //为了方便查询超时的计时器
    std::unordered_map<int64_t , Timer*>active_timer_;       //这是为了方便进行删除的容器，以定时器唯一的标识符sequeue为key
    std::unordered_map<int64_t , Timer*>cancelingTimer_;     //为了解决计时器正在执行却想删除的情况
    std::atomic_bool callingExpiredTimers_;		             //标识定时器是否在执行
public:

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    int64_t addTime(Timer::TimerCallback cb , Timestamp when , double interval);
    void cancel(int64_t timerId);
};
   

#endif