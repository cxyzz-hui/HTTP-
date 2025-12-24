#pragma
#ifndef SERVER_HPP
#define SERVER_HPP

//管理员类，用于管理客户端的各种操作
//把socket类进行二次包装

#include "ThreadPool.hpp"
#include "Callbacks.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"
#include "Socket.hpp"
#include "Epoll.hpp"
#include "util.hpp"
#include "InetAddr.hpp"
#include "Acceptor.hpp"
#include "EventLoop.hpp"
#include "Log/Logger.hpp"
#include "Connection.hpp"
#include "EventLoopThread.hpp"
#include "EventLoopThreadPool.hpp"
#include <memory>
#include <map>

//Accpetor类负责处理新的连接

class Server
{
public:
    using connectionMap = std::map<int , ConnectionPtr>;
private:
    const std::string ipPORT;

    EventLoop *loop_;
   // Socket *serv_sock_;
   // Channel *serv_channel_;   

    std::unique_ptr<Acceptor> acceptor_;
    connectionMap connections_;

    //Buffer inputBuffer_;

    //管理从线程池的操作
    std::atomic_int32_t started_;
    std::unique_ptr<EventLoopThreadPool>loop_threadloop_;
    std::unique_ptr<ThreadPool> compute_threadpool_;	//先添加的，计算线程池

    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    WriteCompleteCallback writeCompleteCallback_;

public:
    
    Server(const InetAddr& serverAddr , EventLoop * eventloop);
    ~Server();

    void start(int IOThreadNum = 0 , int threadNum = 0);

	void setConnectionCallback(const ConnectionCallback& cb)
	{
		connectionCallback_ = cb;
	}
	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{
		writeCompleteCallback_ = cb;
	}

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    EventLoop* getLoop() const { return loop_; }

    //void handleEvent(Channel *ch);
    //void new_Connection(Socket * serv_sock);


private:
    void new_Connection(int sockfd , const InetAddr &peerAddr);
    //去除连接的函数
    void removeConnection(const ConnectionPtr& conn);
    
    void removeConnectionInLoop(const ConnectionPtr& conn);
    // 在对应的loop中移除，不会发生多线程的错误
};



#endif