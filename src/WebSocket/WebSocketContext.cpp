#include "sha1.hpp"
#include "base64.hpp"
#include "WebSocketContext.hpp"

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

WebSocketContext::WebSocketContext()
:Status(WebSocketSTATUS::kUnconnect)
{

}

WebSocketContext::~WebSocketContext()
{

}

void WebSocketContext::praseData(Buffer* buf , Buffer* output)
{
    RequestPacket.decodeFrame(buf , output);
}

void WebSocketContext::handleShared(Buffer* buf , const std::string key_server)
{
    //固定的协议内容
    //buf->append("HTTP/1.1 101 Switching Protocols\r\n");
	//buf->append("Connection: upgrade\r\n");
	//buf->append("Sec-WebSocket-Accept: ");

    buf->append("HTTP/1.1 101 Switching Protocols\r\n");
    buf->append("Upgrade: websocket\r\n");      // 先写 Upgrade
    buf->append("Connection: Upgrade\r\n");     // Connection 值应该是 Upgrade（大写U）
    buf->append("Sec-WebSocket-Accept: ");

    std::string server_key = key_server;
    server_key += MAGIC_KEY;

    //计算哈希，确保正确的连接
    SHA1 sha;
    unsigned int message_digtal[5];
    sha.Reset();
    sha << server_key.c_str();

    sha.Result(message_digtal);             //获取sha1的哈希值
    //for(int i = 0; i < 5; i++) message_digtal[i] = htonl(message_digtal[i]);

    server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digtal), 20);

    buf->append(server_key);
	//buf->append("Upgrade: websocket\r\n\r\n");
    buf->append("\r\n\r\n");

    Status = WebSocketSTATUS::kHandsharked; 
}