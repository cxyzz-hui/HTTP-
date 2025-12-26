#pragma
#ifndef ACCEPTOR_HPP

/*
    对于一个简单的服务器，他有接受连接和处理事件两部分构成
    二这个类专门来处理接受新链接这一部分
    它是内部类，外界是看不到的，供Server类使用的，Acceptor对象的生命周期由Server控制。

    该类应该有个sockfd,其是服务器进行监听的serverFd。
    那也应该有一个channel，把该channel添加到epoll实例中，该fd被激活时，该channel的handleEvent()就会执行，即是调用对应的回调函数。
    那该回调函数是什么？很明显，这是进行监听的serverFd，那就是来建立客户端连接。
*/

#include <functional>
#include "InetAddr.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"

class EventLoop;

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd , const InetAddr&)>;
private:
    NewConnectionCallback newConnectionCallback_;    
    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;

    //是否启用了监听
    bool listen_;
public:

    //传入的参数一是监听某个端口的所有网卡 ，第二个是因为注册这个操作要加入这个服务器的事件循环·1中 
    Acceptor(const InetAddr& listenAddr , EventLoop *eventloop);
    ~Acceptor();

    void setNewconnectionCallback(const NewConnectionCallback &cb) {newConnectionCallback_ = cb;}
    void listen();

private:
    //处理读事件
    void handleRead();
};



#endif