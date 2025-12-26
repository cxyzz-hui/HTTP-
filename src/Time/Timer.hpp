#pragma
#ifndef TIMER_HPP
#define TIMER_HPP

#include <cstdio>
#include <atomic>
#include <functional>
#include "Timestamp.hpp"

//对应着一个超时的Timestamp，本质上是一个超时类

class Timestamp;

class Timer
{ 
public:
    using TimerCallback = std::function<void()>;
    Timer(TimerCallback cb , Timestamp when , double interval)
        :callback_(std::move(cb))
        ,expiration_(when)
        ,interval_(interval)
        ,repeat_(interval > 0.0)
        ,sequence_(num_create_++)
    {}


    void run() const {callback_(); }
    Timestamp expiration()const {return expiration_;}
    bool repeat()const {return repeat_;}
    int64_t sequence() const {return sequence_;}
    void restart(Timestamp now);
    static int64_t num_Created() {return num_create_;}

private:
    const TimerCallback callback_;       //定时器执行的事件
    Timestamp expiration_;               //超时事件
    const double interval_;              //时间间隔
    const bool repeat_;                   //用于判断定时器是否是循环使用的
    const int64_t sequence_;             //判断定时器的

    static std::atomic_int64_t num_create_;
};


#endif