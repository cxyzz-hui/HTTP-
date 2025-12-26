#pragma
#ifndef WEBSOCKETCONTEXT_HPP
#define WEBSOCKETCONTEXT_HPP

#include "WebSocketPacket.hpp"
#include "../Buffer.hpp"

//这个类专门用来处理连接

class WebSocketContext
{
public:
    enum WebSocketSTATUS
    {
        kUnconnect, kHandsharked
    };

    WebSocketContext();
    ~WebSocketContext();

    void handleShared(Buffer* buf , const std::string key_server);

    //解析数据
    void praseData(Buffer* buf , Buffer* output);
    void reset() {RequestPacket.reset(); }

    void SetWebScoketHandShared() { Status = WebSocketContext::WebSocketSTATUS::kHandsharked; }
    WebSocketSTATUS GetWebSocketSTATUS() { return Status; }

    uint8_t GetRequestOpcode() const { return RequestPacket.Get_opcode(); }

private:

    WebSocketSTATUS Status;
    WebSocketPacket RequestPacket;
};


#endif