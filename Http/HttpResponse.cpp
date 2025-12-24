#include "HttpResponse.hpp"
#include "../Buffer.hpp"

HttpResponse::HttpResponse(bool state)
:StateCode(HttpStateCode::KUnknown)
,CloseConnection(state)
{

}

//将HTTP响应转换为符合HTTP协议格式
//按照HTTP协议对HttpResponse对象进行格式化输出到Buffer中
//注意:HTTP请求报文的格式是固定的
void HttpResponse::appendToBuffer(Buffer *output) const
{
    //响应行
    std::string buf = "HTTP/1.1 " + std::to_string(static_cast<int>(StateCode));
    output->append(buf);
    output->append(StatusMessage);
    output->append("\r\n");

    //响应头部
    if(CloseConnection)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        output->append("Connection: Keep-Alive\r\n");
		buf = "Content-Length:" + std::to_string(boby.size()) + "\r\n";
		output->append(buf);
    }

    //响应头:说明这个是什么内容

    for(const auto& header : Header)
    {
        buf = header.first + ": " + header.second + "\r\n";
		output->append(buf);
    }

    output->append("\r\n");	//空行
	output->append(boby);	//响应体
}