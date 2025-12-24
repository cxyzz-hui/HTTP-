#include "HttpServer.hpp"
#include "HttpRequest.hpp"
#include "HttpContext.hpp"
#include "HttpResponse.hpp"     

//设置回调函数的默认参数
void DefaultHttpCallback(const HttpRequest& Request , HttpResponse* Response)
{
    Response->SetStateCode(HttpResponse::HttpStateCode::K404NotFound);
    Response->SetStatusMessage("Not Found");
    Response->SetCloseConnetion(true);
}

HttpServer::HttpServer(EventLoop* loop , const InetAddr& Inet)
:server(Inet , loop)
//,httpCalback([this](const HttpRequest& Request , HttpResponse* Response) { DefaultHttpCallback(Request , Response); })
{
    httpCallback = [](const HttpRequest& req, HttpResponse* resp) {
        DefaultHttpCallback(req, resp);
    };
    server.setConnectionCallback([this](const ConnectionPtr& conn) 
    { 
        onConnection(conn); 
    });
    server.setMessageCallback([this](const ConnectionPtr& conn , Buffer* buf) 
    {
        onMessage(conn , buf); 
    });
}

void HttpServer::start(int numThread)
{
    server.start(numThread);
}

void HttpServer::onConnection(const ConnectionPtr& conn)
{
    if(conn->connected())
    {
        
        conn->SetContext(HttpContext());
		//测试使用，用来测试绑定不符合的类型,为了长连接做准备
    }

}

//消息处理函数
void HttpServer::onMessage(const ConnectionPtr& conn , Buffer* buf)
{
    auto Context = std::any_cast<HttpContext>(conn->GetMutableContext());
    if(!Context)
    {
        printf("context kong...\n");
        LOG_ERROR << "context is bad\n";
        return;
    }

    if(!Context->ParseBuffer(buf))
    {
        //请求失败,发送bad页面
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
        printf("http 400\n");
    }

    if(Context->GetAll()) 
    {
        onRequest(conn, Context->Request());
        Context->Reset();
        //一旦请求处理完毕，重置context，因为HttpContext和Connection绑定了，我们需要解绑重复使用
	}
    
}

void HttpServer::onRequest(const ConnectionPtr& conn , const HttpRequest& Request)
{
    const std::string connection = Request.GetHeader("Connection");
    bool close = connection == "close" || (Request.GetVersion() == HttpRequest::Version::Http10 && connection != "Keep-Alive");
    HttpResponse Response(close);

    //执行用户注册的回调函数
    httpCallback(Request , &Response);

    Buffer buf;
    Response.appendToBuffer(&buf);
    conn->send(&buf);
    if(Response.CloseConnetion_())
    {
        conn->shutdown();
    }

}
