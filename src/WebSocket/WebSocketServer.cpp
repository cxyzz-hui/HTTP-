#include "WebSocketServer.hpp"
#include "WebSocketContext.hpp"

#include "../Http/HttpRequest.hpp"
#include "../Http/HttpContext.hpp"

#include "../Buffer.hpp"

//默认回调函数
void DefaultWebSocketCallback(const Buffer* buf , Buffer* sendbuf)
{
    //进行echo，原数据返回
	sendbuf->append(buf->peek(), buf->readableBytes());
}

WebSocketServer::WebSocketServer(EventLoop* loop , const InetAddr& listenAddr)
:server(listenAddr , loop)
,webSocketCallback(DefaultWebSocketCallback)
{
    //设置回调函数
    server.setConnectionCallback([this](const ConnectionPtr& conn) {onConnetion(conn); });
    server.setMessageCallback([this](const ConnectionPtr& conn , Buffer* buf) { onMessage(conn , buf); });
}

WebSocketServer::~WebSocketServer()
{

}

void WebSocketServer::start(int numThread)
{
    server.start(numThread);
}

void WebSocketServer::onConnetion(const ConnectionPtr& conn)
{
    //为每个连接创建相应的WebSocketContext对象，为后续的WebSocket通信做准备
    if(conn->connected())
    {
        conn->SetContext(WebSocketContext());
    }
}

//业务函数
void WebSocketServer::onMessage(const ConnectionPtr& conn , Buffer* buf)
{
    //获取之前创建的WebSocketContext类
    auto Context = std::any_cast<WebSocketContext>(conn->GetMutableContext());
    if(!Context)
    {
        printf("Context is empyt\n");
        LOG_ERROR << "Context is bad\n";
    }

    if(Context->GetWebSocketSTATUS() == WebSocketContext::WebSocketSTATUS::kUnconnect)
    {
        printf("进入了WebSocketServer::onMessage\n");
        //链接成功开始解析HTTP请求
        HttpContext http;
        if(!http.ParseBuffer(buf))
        {
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }

        //检查是否是完整的HTTP请求
        if(http.GetAll())
        {
            auto httpRequest = http.Request();
            
            //检查HTTP是否为合法的WebSocket协议
            if (httpRequest.GetHeader("Upgrade") != "websocket" ||
				httpRequest.GetHeader("Connection") != "Upgrade" ||
				httpRequest.GetHeader("Sec-WebSocket-Version") != "13" ||
				httpRequest.GetHeader("Sec-WebSocket-Key") == "")
            {
                //如果不是WebSocket协议的话，发送bad请求，之后关闭连接
                conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
                conn->shutdown();
            }

            //建立WebSocket连接
            Buffer handsharedbuf;
            Context->handleShared(&handsharedbuf , http.Request().GetHeader("Sec-WebSocket-Key"));
            conn->send(&handsharedbuf);
            Context->SetWebScoketHandShared();

            buf->retrieveAll();
        }
    }
    else
    {
        handleData(conn , Context , buf);
    }

}

//处理函数
void WebSocketServer::handleData(const ConnectionPtr& conn, WebSocketContext* websocket, Buffer* buf)
{   
    Buffer DataBuf;
    //先解析数据
    websocket->praseData(buf , &DataBuf);

    //创建帧包变量
    WebSocketPacket SocketPacket;
    int opcode = websocket->GetRequestOpcode();

    //根据给定的选项来给这个包定义变量
	switch (opcode)
	{
	case WSOpcodeType::WSOpcode_Continue:
		SocketPacket.Set_opcode_(WSOpcodeType::WSOpcode_Continue);
		printf("WebSocketEndpoint - recv a Continue opcode.\n");
		break;

	case WSOpcodeType::WSOpcode_Text:
		SocketPacket.Set_opcode_(WSOpcodeType::WSOpcode_Text);
		printf("WebSocketEndpoint - recv a Text opcode.\n");
		break;

	case WSOpcodeType::WSOpcode_Binary:

		SocketPacket.Set_opcode_(WSOpcodeType::WSOpcode_Binary);
		printf("WebSocketEndpoint - recv a Binary opcode.\n");
		break;
	case WSOpcodeType::WSOpcode_Close:

		SocketPacket.Set_opcode_(WSOpcodeType::WSOpcode_Close);
		printf("WebSocketEndpoint - recv a Close opcode.\n");
		break;
	case WSOpcodeType::WSOpcode_Ping:

		SocketPacket.Set_opcode_(WSOpcodeType::WSOpcode_Pong);	//进行心跳响应
		printf("WebSocketEndpoint - recv a Ping opcode.\n");
		break;
	case WSOpcodeType::WSOpcode_Pong:		//表示这是一个心跳响应(pong)，那就不用回复了

		printf("WebSocketEndpoint - recv a Pong opcode.\n");
		return;
	default:
		LOG_INFO << "WebSocketEndpoint - recv an unknown opcode.\n";
		return;
	} 

    Buffer Sendbuf;
    if(opcode != WSOpcodeType::WSOpcode_Close && opcode != WSOpcodeType::WSOpcode_Ping && WSOpcodeType::WSOpcode_Pong)
    {
        //如果满足了以上条件那就意味着需要触发回调函数来处理数据了
        webSocketCallback(&DataBuf , &Sendbuf);
    }

    Buffer frameBuf;
    SocketPacket.encodeFrame(&frameBuf , &Sendbuf);      //封装帧

    //发送并重置
    conn->send(&frameBuf);        
    websocket->reset();
}