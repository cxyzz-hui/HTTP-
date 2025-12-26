#include"util.hpp"
#include "Log/Logger.hpp"

#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

void perror_if(bool condtion, const char* errorMessage)
{
	if (condtion) 
	{
		perror(errorMessage);
		exit(1);
	}
}

void sockets::setReuseAddr(int sockfd)
{
	int opt = 1;
	::setsockopt(sockfd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));
}

void sockets::setNonblock(int sockfd)
{
	int flag = fcntl(sockfd , F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(sockfd , F_SETFL , flag);
}

//关闭写功能
void sockets::shutdownWrite(int sockfd)
{
	//shutdown:半关闭状态，SHUT_WR:值关闭写的功能，保留读功能
	if(::shutdown(sockfd , SHUT_WR) < 0)
	{
		printf("sockets::shutdownWrite error in 26 line at util.cpp\n");
	}
}

//获得sockfd的错误码
int sockets::getSocketError(int sockfd)
{
	int optval;
	//将optval强转为socklen_t类型
	socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

	if(::getsockopt(sockfd , SOL_SOCKET , SO_ERROR , &optval , &optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
	struct sockaddr_in localAddr;
	socklen_t addrLen = static_cast<socklen_t>(sizeof(localAddr));

	//getsockname:获取sockfd绑定的信息
	if(::getsockname(sockfd , (struct sockaddr *)&localAddr , &addrLen) < 0)
	{
		printf("getsockname error in 58 line at util.cpp\n");
	}
	
	return localAddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
	struct sockaddr_in peerAddr;
	socklen_t addrlne = static_cast<socklen_t>(sizeof(peerAddr));

	if(getsockname(sockfd , (struct sockaddr *)&peerAddr , &addrlne) < 0)
	{
		printf("getsockname error in 74 line at util.cpp\n");
	}
	return peerAddr;
}

//获取运行程序的主机名
std::string ProcessInfo::hostHome()
{
	char buf[256];
	//gethostname:接受主机名的缓冲区
	if(::gethostname(buf , sizeof(buf)) == 0)
	{
		buf[sizeof(buf) - 1] = '\0';
		return buf;
	}
	else
	{
		return "UnKnownHost";
	}

}

pid_t ProcessInfo::pid()
{
	return ::getpid();
}