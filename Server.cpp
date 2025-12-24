#include "Server.hpp"
#include "util.hpp"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

const int READ_BUFFER = 1024;

void setNonblock(int sockfd)
{
	int flag = fcntl(sockfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flag);
}

Server::Server(const InetAddr& listenAddr , EventLoop *eventloop) 
: loop_(eventloop) 
, acceptor_(std::make_unique<Acceptor>(listenAddr , loop_))       //创建智能指针管理客户端的创建
, ipPORT(listenAddr.toIpPort())
, loop_threadloop_(std::make_unique<EventLoopThreadPool>(loop_))
, compute_threadpool_(std::make_unique<ThreadPool>())
, started_(0)
{
    //serv_sock_->bind(&listenAddr);
    //serv_sock_->listen();
    //serv_sock_->setNonblock();

    //给服务器分配个Channel类
    //serv_channel_ = new Channel(loop_ , serv_sock_->fd());

    //回调 = 预先注册的函数 + 事件触发时自动调用
    //这里先预先注册一个cb函数作为回调。之后遇见new——Connection执行回调函数
    //执行函数在Channel那个类里面
    //这里先预先注册个函数，等以后调用handleEvent就会处理了
    auto cb = [this](int sockfd , const InetAddr& peerAddr){new_Connection(sockfd , peerAddr); };
    acceptor_->setNewconnectionCallback(cb);

}

Server::~Server()
{
    //不一定进入这个for循环，因为进入之前可能里面就没有数据了
    for(auto& item : connections_)
    {
        ConnectionPtr conn(item.second);
        //释放对象，并将其设为空指针
        item.second.reset();

        //取消连接
        conn->connectDestroyed();
    }
}

void Server::start(int IOThreadNum , int threadNum)
{
    if(started_++ == 0)
    {
        loop_threadloop_->setThreadNum(IOThreadNum);
        compute_threadpool_->start(threadNum);
        loop_threadloop_->start();
        acceptor_->listen();
    }
}

/*
void Server::handleEvent(Channel *ch)
{
    int fd = ch->FD();
    char buf[READ_BUFFER];

    //清空并读取数据
    memset(buf , 0 , sizeof(buf));

    //更新一下缓冲区的相关操作，缓冲Buf类里面的东西
    int saveErrno;
    auto len = inputBuffer_.readFd(fd , &saveErrno);
    if(len > 0)
    {   
        auto msg = inputBuffer_.retrieveAllAsString().c_str();
        printf("%s:%d" , fd , msg);
        write(fd , msg , len);
    }
    else if(len == 0)
    {
        printf("client fd %d disconnectde" , fd);

        //下线了把数据移除
        loop_->removeChannel(ch);
        close(fd);
    }
    else
    {
        perror_if(len , "read error in 64 line at Servre.cpp");
    }

}
*/

void Server::new_Connection(int sockfd , const InetAddr &peerAddr)
{
    //设置为非堵塞状态
    //sockets::setNonblock(sockfd);

    //选择一个子Reactor循环线程
    EventLoop* ioLoop = loop_threadloop_->getNextLoop();

    InetAddr localAddr(sockets::getLocalAddr(sockfd));

    auto conn = std::make_shared<Connection>(ioLoop , sockfd , localAddr , peerAddr);
    //将这个链接放到map里面方便管理
    connections_[sockfd] = conn;

    //注册回调函数
    conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback([this](const ConnectionPtr& connection) {removeConnection(connection);});
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    //到本线程后再创建链接
    ioLoop->runInLoop([conn](){conn->connectEstablished();});

    //conn->connectEstablished();
    //建立连接
}

void Server::removeConnection(const ConnectionPtr& conn)
{
    /*去取这个套接字在map的存在
    auto n = connections_.erase(conn->fd());
    assert(n == 1);
    conn->connectDestroyed();
    */

    loop_->runInLoop([conn , this](){ removeConnectionInLoop(conn);});
}

void Server::removeConnectionInLoop(const ConnectionPtr &conn)
{
    connections_.erase(conn->fd());
    auto ioLoop = conn->getLoop();

    ioLoop->queueInLoop([conn](){conn->connectDestroyed();});
}


/*void Server::new_Connection(Socket* serv_sock)
{
    //对客户端相应结构体进行初始化
    //和构造函数差不多
    InetAddr Client_addr;
    Socket* client_sock = new Socket(serv_sock->accept(&Client_addr));
    client_sock->setNonblock();
    //设置非堵塞

    Channel* channel = new Channel(loop_ , client_sock->fd());

    auto cb = [this , channel](){handleEvent(channel);};
    channel->SetCallback(cb);
    channel->enableReading();
}*/