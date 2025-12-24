#pragma
#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

//处理事件循环的类
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <sys/eventfd.h>

#include "Epoll.hpp"
#include "Channel.hpp"
#include "Timestamp.hpp"
#include "Callbacks.hpp"

//表明这俩玩意是两个类
class TimerQueue;
class Channel;
class Epoll;

class EventLoop
{
public:
    using Functor = std::function<void()>;
    using channelList = std::vector<Channel*>;
public:
    EventLoop();
    ~EventLoop();

    //循环的函数
    void loop();

    //更新和移除Channel(传输数据的管道)类的函数
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    bool isInLoopThread()const {return threadId_ == std::this_thread::get_id();}
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    // 用来唤醒loop所在线程  向wakeupfd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
    void wakeup();

    void assertInLoopThread()
	{
		if (!isInLoopThread()) 
        {
			printf("not in this loopThread\n");
			//会出错，现在还没有处理
		}
	}

    void quit();

    //这里提供一些接口供其他类可以使用Timerstamp里面的相应函数
    //这里分别是执行和取消计时器的一些相关函数
    int64_t runAt(Timestamp time , TimerCallback cb);
    int64_t runAfter(double delay_seconds , TimerCallback cb);
    int64_t runEntry(double interval_seconds , TimerCallback cb);
    void cancel(int64_t timerId);

private:

    //执行回调函数的函数
    void doPendingFunctor();
    void handleRead();

    std::thread::id threadId_;
    std::atomic_bool quit_;
    
    //判断当前循环是否需要回调操作
    std::atomic_bool callingPendingFunctors_;
    
    std::unique_ptr<Epoll> ep_;
    std::vector<Channel*>activeChannel;

    int wakeupfd_;
    std::unique_ptr<Channel>wakeupChannel;

    //保护最后一个回调函数的的线程安全
    std::mutex mutex_;
    std::vector<Functor>pendingFunctors_;

    std::unique_ptr<TimerQueue>TimeQueue_;      //使EventLoop可以操控定时器队列
};




#endif