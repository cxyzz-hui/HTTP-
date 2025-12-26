#pragma
#ifndef UTIL_HPP
#define UTIL_HPP

#include <arpa/inet.h>
#include <string>

//打印错误日志
void perror_if(bool condtion, const char* errorMessage);

namespace sockets
{
    void setReuseAddr(int sockfd);

    void setNonblock(int sockfd);

    void shutdownWrite(int sockfd);

    int getSocketError(int sockfd);

    struct sockaddr_in getLocalAddr(int sockfd);
    struct sockaddr_in getPeerAddr(int sockfd);

};

namespace ProcessInfo
{
    std::string hostHome();

    pid_t pid();
};

#endif