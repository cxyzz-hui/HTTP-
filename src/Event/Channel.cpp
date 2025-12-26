#include "Channel.hpp"
#include "EventLoop.hpp"

#include <memory>
#include <vector>

Channel::Channel(EventLoop *loop , int fd) : loop_(loop) , fd_(fd) , events_(0) , revents_(0) , isInEpoll_(false) , tied(false) {}

void Channel::setEvent(int events)
{
    events_ = events;
}

int Channel::Event() const
{
    return events_;
}

int Channel::Revent() const
{
    return revents_;
}

bool Channel::isInEpoll()
{
    return isInEpoll_ == true;
}

void Channel::setInEpoll(bool in)
{
    isInEpoll_ = in;
}

int Channel::FD()
{
    return fd_;
}


void Channel::setRevents(int revents)
{
    revents_ = revents;
}

//channel得到poller通知以后，处理事件的
void Channel::handleEvent()
{
   
    if(tied)
    {
        //如果绑定了那就执行
        auto guard = tie_.lock();
        if(guard)
        {
            handleEventWithGuard();
        }
    }
    else
    {
        //这个else里面是用来建立连接的，因为开始建立连接的时候tied_是false,是连接建立后开始通信tied_才为true
        handleEventWithGuard();
    }

}
void Channel::remove()
{
	loop_->removeChannel(this);
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::tie(const std::shared_ptr<void>&obj)
{
    //传递指针对象(不影响其生命周期，防止运行回调函数的时候直接被截断)
    tie_ = obj;
    tied = true;
}

//处理事件可能出现的所有情况，然后执行时间
void Channel::handleEventWithGuard()
{
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) 
    {
        //当事件为挂起并没有可读事件时
		if (closeCallback_) 
        {
			printf("channel closeCallback\n");
			closeCallback_();
		}
	}
	if (revents_ & EPOLLERR) 
    {
		if (errorCallback_)
		errorCallback_();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) 
    { 
        //关于读的事件 
		if (readCallback_)
		readCallback_();
	}
	if (revents_ & EPOLLOUT) 
    {   
        //关于写的事件
		if (writeCallback_)
		writeCallback_();
	}
}