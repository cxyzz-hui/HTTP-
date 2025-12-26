#pragma
#ifndef WEBSOCKETSERVER_HPP
#define WEBSOCKETsERVER_HPP

#include <functional>

#include "../Server.hpp"
#include "WebSocketPacket.hpp"

//和HttpServer的类功能差不多

class WebSocketContext;
class HttpResponse;
class HttpRequest;


class WebSocketServer
{
public:
	using WebSocketCallback = std::function<void(const Buffer* , Buffer*)>;

    WebSocketServer(EventLoop* loop , const InetAddr& listenAddr);
    ~WebSocketServer();

    EventLoop* GetLoop() const { return server.getLoop(); }

    void SetWebScoketCallback(const WebSocketCallback cb) { webSocketCallback = cb; }

    void start(int numThread);

private:

    void onConnetion(const ConnectionPtr& conn);	//连接到来的回调函数
	void onMessage(const ConnectionPtr& conn, Buffer* buf);	//消息到来的回调函数
	void handleData(const ConnectionPtr& conn, WebSocketContext* websocket, Buffer* buf);

    WebSocketCallback webSocketCallback;
    Server server;

};


#endif