#include "Acceptor.hpp"
#include "util.hpp"

Acceptor::Acceptor(const InetAddr& listenAddr , EventLoop* eventloop) 
: loop_(eventloop) 
, acceptSocket_(Socket()) 
, acceptChannel_(loop_ , acceptSocket_.fd()) 
, listen_(false)
{
    //设置端口复用
    sockets::setReuseAddr(acceptSocket_.fd());

    //绑定IP地址
    acceptSocket_.bind(listenAddr);

    //注册回调函数
    auto cb = [this]() {handleRead();};
    //将处理函数加入回调列表，以便以后进行处理
    acceptChannel_.SetReadCallback(cb);

    //开始监听
    //this -> listen();
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    //启动监听，设置关心事件
    acceptSocket_.listen();
    acceptChannel_.enableReading();
    listen_ = true;
    printf("开始监听  listenfd= %d\n",acceptSocket_.fd());
}


void Acceptor::handleRead()
{
    //接受新的连接
	InetAddr peerAddr;
	int connfd = acceptSocket_.accept(&peerAddr);
	if (connfd >= 0) 
    {
		if (newConnectionCallback_) 
        {
			newConnectionCallback_(connfd , peerAddr);
		}
	}
    else
    {
        printf("accpet error\n");
    }
}