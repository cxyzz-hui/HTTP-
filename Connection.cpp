#include "Connection.hpp"
#include "Channel.hpp"
#include "Socket.hpp"
#include "util.hpp"
#include <cassert>


Connection::Connection(EventLoop* loop , int sockfd , const InetAddr& localAddr , const InetAddr& peerAddr) : loop_(loop) 
, state_(StatE::kConnecting) 
, socket_(std::make_unique<Socket>(sockfd)) 
, channel_(std::make_unique<Channel>(loop , sockfd))
, localAddr_(localAddr)
, peerAddr_(peerAddr)
{
    //设置好读写关，三者的回调函数
    channel_->SetReadCallback([this](){handleRead();});
    channel_->setWriteCallback([this](){handleWrite();});
    channel_->setCloseCallback([this](){handleClose();});
    channel_->setErrorCallback([this](){handleError();});

    //channel_->enableReading();
}

Connection::~Connection()
{
    printf("Connection::dtor at  fd=%d  state=%d\n", channel_->FD(), static_cast<int>(state_));
}

void Connection::send(Buffer* message)
{
    send(message->peek() , message->readableBytes());
    message->retrieveAll();
}

void Connection::send(const char* message, size_t len)
{
    //自从我们进入多线程以后，我们这里就要判断一下这里是不是在我们的当前线程
    if(state_ == StatE::kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message , len);
        }
        else
        {
            loop_->runInLoop([this , message , len](){sendInLoop(message , len);});
        }
    }

}

//发送数据的函数
void Connection::send(const std::string& message)
{
    send(message.data() , message.size());
}

//连接确定与摧毁
void Connection::connectEstablished()
{
    assert(state_ == StatE::kConnecting);
    setState(StatE::kConnected);
    channel_->tie(shared_from_this());
 
    channel_->enableReading();
    //调用回调函数
    connectionCallback_(shared_from_this());
}

void Connection::connectDestroyed()
{
    if(state_ == StatE::kConnected)
    {
        setState(StatE::kDisconnected);
        connectionCallback_(shared_from_this());
        channel_->disableAll();
        
    }

    channel_->remove();
}

void Connection::shutdown()
{
    if(state_ == StatE::kConnected)
    {
        setState(StatE::kDisconnecting);
        
        //这里要在本线程执行关闭操作不然会被其他线程影响然后直接截断报错
        loop_->runInLoop([this]() { shutdownInLoop(); });
    }
}

void Connection::shutdownInLoop()
{
	if (!channel_->isWrite())  // 说明当前outputBuffer中的数据已经全部发送完成
	{
		
		sockets::shutdownWrite(fd());   // 关闭写端 ,能触发EPOLLHUP,也会触发EPOLLIN
	}
}

void Connection::forceClose()
{
    if(state_ == StatE::kConnected || state_ == StatE::kDisconnecting)
    {
        //如果在运行直接强制关闭
        setState(StatE::kDisconnecting);
    
        //使用shared_from_this自动生成智能指针防止在异步过程中对象被销毁而导致崩溃
		loop_->queueInLoop([this]() { shared_from_this()->forceCloseInLoop(); });
    }
}

void Connection::forceCloseInLoop()
{
    //在当前线程关闭
    loop_->assertInLoopThread();
    if(state_ == StatE::kConnected || state_ == StatE::kDisconnecting)
    {
        setState(StatE::kDisconnecting);
        handleClose();
    }
}

//处理事假
void Connection::handleRead()
{
    //读事件用input(输入)
    int saveErrno = 0;
    int n = inputBuffer_.readFd(fd() , &saveErrno);

    if(n > 0)
    {
        //作用：通知回调函数来处理数据
        messageCallback_(shared_from_this(), &inputBuffer_);	//这个是用户设置好的函数
        //新添加的，没有这句代码的话，那readindex可能就没有变化，那读取的数据就会包含上一次的
		inputBuffer_.retrieve(inputBuffer_.readableBytes());	//messageCallback_中处理好读取的数据后，更新readerIndex位置
    }
    else if(n == 0)
    {
        //if(state_ == StatE::kConnected || state_ == StatE::kDisconnecting)
        //客户端关闭了链接
        handleClose();
    }
    else
    {
        handleError();
    }

}

void Connection::handleWrite()
{
    if(!channel_->isWrite())
    {
        printf("connection fd = %d is down ,no more writing\n" , channel_->FD());
        return;
    }

    //获取读取数据的长度
    auto n = ::write(fd() , outputBuffer_.peek() , outputBuffer_.readableBytes());
    if(n > 0)
    {
        outputBuffer_.retrieve(n);
        if(outputBuffer_.readableBytes() == 0)
        {
            //如果数据全部接受完成那么就要取消写事件
            channel_->disableWriting();
        }
        else
        {
            printf("read to write more date\n");
        }
    }
    else
    {
        printf("handleWrite error\n");
    }
}

/*
    这里有个bug还没排查，具体体现为客户端连接至服务器退出不知道为啥会退两次
*/

void Connection::handleClose()
{
    //处理关闭事件
    //如果它之前不是这辆状态，那么它肯定错误了，直接截断

    assert(state_ == StatE::kConnected || state_ == StatE::kDisconnecting);

    //设置状态，关闭所有事件并且关闭智能指针
    setState(StatE::kDisconnected);
    channel_->disableAll();

    ConnectionPtr guardThis(shared_from_this());

    connectionCallback_(guardThis);
    //printf("Connection::handleClose() guardThis(shared_from_this())后 user_count= %ld\n", guardThis.use_count());
    closeCallback_(guardThis);

    //closeCallback_就是Server::removeConnection()函数
}

void Connection::handleError()
{
    int err = sockets::getSocketError(channel_->FD());
    printf("Connection::handleError err : %d" , err);
}

void Connection::sendInLoop(const char *message , size_t len)
{

    //如果已经断开连接的话就不执行了
    if(state_ == StatE::kDisconnected)
    {
        return;
    }

    bool faultError = false;
    ssize_t nwrote = 0;
    size_t remaining = len;

    //如果缓冲区没有数据，并且现在没有写事件发生就可以直接发送
    if(!channel_->isWrite() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(fd() , message , len);
        if(nwrote >= 0)
        {
            //如果写入的字节数大于0
            remaining = len - nwrote;
            if(remaining == 0)
            {
                //数据发送完毕，提醒服务器一下让下一个用户继续发送消息
                if(writeCompleteCallback_)
                {
                    // 关键：调用应用层注册的回调函数
                    //处理的应用层
                    writeCompleteCallback_(shared_from_this());
                }
            }
        }
        else
        {
            //nwrote < 0去情况下
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }

    assert(static_cast<size_t>(remaining) <= len);
    //如果发送成功并且还有内容的情况下，再尝试进行下一次发送
    if(!faultError && remaining > 0)
    {
        outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
        if(!channel_->isWrite())
        {
            channel_->enableWriting();
        }
    }

}