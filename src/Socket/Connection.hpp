#pragma
#ifndef CONNECTION_HPP
#define CONNECTION_HPP

//在经过之前我们把Socket类的连接功能交给Accpetor类之后，我们这个Connection类就要用来管理连接的TCP连接并且进行相关处理的操作了
//

#include "../Callbacks.hpp"
#include "Socket.hpp"
#include "Buffer.hpp"
#include <memory>
#include <any>

class EventLoop;
class Channel;

//后面那一串继承的作用是让成员函数可以安全的获取自身的share_ptr(智能指针)，为了防止在异步多线程过程中多个回调函数执行中直接被销毁从而导致的程序崩溃
class Connection:public std::enable_shared_from_this<Connection>
{
public:
    //这里用Class主要是为了防止命名空间冲突
    enum class StatE {kDisconnected , kConnecting , kConnected , kDisconnecting};
    //这四个操作分别代表着已经断开，正在连接，连接中，正在断开
public:
    Connection(EventLoop* loop , int sockfd , const InetAddr& localAddr , const InetAddr& peerAddr);
    ~Connection();

    //获得所区循环
    EventLoop* getLoop()const {return loop_;}
    //设置回调函数
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }
    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

    
    //判断Connection的状态
    bool connected() const {return state_ == StatE::kConnected;}
    bool disconnected() const {return state_ == StatE::kDisconnected;}

    //设置状态

    //发送数据
    void send(Buffer* message);
    void send(const char *message , size_t len);
    void send(const std::string& message);

    //确定链接状态
    void connectEstablished();
    void connectDestroyed();

    //强制关闭和半关闭状态
    void shutdown();
    void forceClose();

	const InetAddr& localAddress() const { return localAddr_; }
	const InetAddr& peerAddress() const { return peerAddr_; }

    //获取信息的基本函数
    Buffer* inputBuffer()
    {
        return &inputBuffer_;
    }

    Buffer* outputBuffer()
    {
        return &outputBuffer_;
    }

    int fd()const {return socket_->fd();}

    void SetContext(const std::any& context) { Context = context; }
    std::any* GetMutableContext() { return &Context; }
private:
    void setState(StatE state) {state_ = state;}

    //私有成员来处理各种事件
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void forceCloseInLoop();
    void shutdownInLoop();
    void sendInLoop(const char *message , size_t len);
private:

    EventLoop *loop_;
    
    StatE state_;

    //管理Socket和Channel
    std::unique_ptr<Socket>socket_;
    std::unique_ptr<Channel>channel_;

    const InetAddr localAddr_;
    const InetAddr peerAddr_;

    //声明回调函数
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    ConnectionCallback connectionCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    //这是接受Http链接的那个类，同时为了方便这里用的any
    std::any Context;
};




#endif