#pragma
#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

//#include "../Server.hpp"
#include "../Socket/Server.hpp"
#include <functional>

class HttpRequest;
class HttpResponse;

class HttpServer
{
public:
    using HttpCallback = std::function<void(const HttpRequest& , HttpResponse*)>;

    HttpServer(EventLoop* loop , const InetAddr& listenAddr);

    void SetHttpCallback(const HttpCallback& cb) { httpCallback = cb; }

    EventLoop* GetEventLoop() const {return server.getLoop(); }

    void start(int nunThread);

private:

    void onConnection(const ConnectionPtr& conn);             //连接到来的回调函数
    void onMessage(const ConnectionPtr& conn , Buffer* buf);  //消息到来的回调函数
    void onRequest(const ConnectionPtr& conn , const HttpRequest& request);

    Server server;
    HttpCallback httpCallback;

};

#endif