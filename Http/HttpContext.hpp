#pragma
#ifndef HTTPCONTEXT_HPP
#define HTTPCONTEXT_HPP

#include "HttpRequest.hpp"

//解析HTTP协议

class Buffer;

class HttpContext
{
public:

    enum class HttpRequestPaseState
    {
        KExpectRequestLine,          //请求行
        KExpectHeader,               //请求头
        KExpectBody,                 //请求体
        KGotAll                      //表示接受完全
    };

    HttpContext() : State(HttpRequestPaseState::KExpectRequestLine) {}

    bool ParseBuffer(Buffer* buf , Timestamp ReceiveTime);              //解析请求的buf
    bool ParseBuffer(Buffer* buf);

    bool GetAll() const {return State == HttpRequestPaseState::KGotAll; }

    void Reset();

    const HttpRequest& Request() const {return Request_; }
    HttpRequest& Request() { return Request_; }

private:
    bool ProcessRequestLine(const char *begin , const char *end);

    HttpRequestPaseState State;         //表示需要处理的状态
    HttpRequest Request_;                 //请求类

};



#endif