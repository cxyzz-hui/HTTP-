#include "HttpContext.hpp"
#include "../Buffer.hpp"

//从接受的数据中，解析出正确的协议信息
bool HttpContext::ParseBuffer(Buffer* buf , Timestamp ReceiveTime)
{
    bool ok = true;
    bool More = true;

    //持续解析协议内容
    //主要状态就是找到分隔符，看看是否有请求数据，如果有就更新状态继续寻找，如果没有就结束状态并且返回结果
    while(More)
    {
        if(State == HttpRequestPaseState::KExpectRequestLine)
        {
            //解析请求行

            //首先我们来查找请求数据中第一次出现\r\n的地方，如果没有，那么这个就没有更多数据了
            const char *crlf = buf->findCRLF();
            if(crlf)
            {
                //这里有\r\n，那么我们调用其他函数来判断里面是否还有数据
                ok = ProcessRequestLine(buf->peek() , crlf);
                if(ok)
                {
                    //解析成功更新数据
                    Request_.SetRecieveTime(ReceiveTime);
                    buf->retrieveUntil(crlf + 2);                   //跳过\r\n到下一行
                    State = HttpRequestPaseState::KExpectHeader;    //更新状态为继续解析
                }
                else
                {
                    More = false;
                }
            }
            else
            {
                More = false;
            }
        }
        else if(State == HttpRequestPaseState::KExpectHeader)
        {
            //请求头的例子:Host: example.com\r\nContent-Type: text/html\r\n\r\n
            //解析请求头
            const char *crlf = buf->findCRLF();     //和之前操作类似
            if(crlf)
            {
                //根据上面的例子我们可以知道当它们二者相同的时候那么意味着请求行结束了,这是因为如果find函数没有找到":"的话，则会返回crlf的文职
                const char *colon = std::find(buf->peek() , crlf , ':');
                if(crlf != colon)
                {
                    Request_.addHeader(buf->peek() , colon , crlf);
                }
                else
                {
                    State = HttpRequestPaseState::KExpectBody;
                }

                buf->retrieveUntil(crlf + 2);
            }
            else
            {
                More = false;
            }

        }
        else if(State == HttpRequestPaseState::KExpectBody)
        {
            //解析请求体
            if(buf->readableBytes())
            {
                //如果还有数据，那么它就是请求体
                Request_.SetQuery(buf->peek() , buf->beginWirte());
            }
            State = HttpRequestPaseState::KGotAll;
            More = false;
        }
    }
    
    return ok;
}

bool HttpContext::ParseBuffer(Buffer* buf)
{
    //和上面的思路差不多，只不过是少了个时间戳
    bool ok = true;
    bool More = true;
    printf("HttpContext::parseReques:buf:\n%s\n",buf->peek());

    while(More)
    {
        if(State == HttpRequestPaseState::KExpectRequestLine)
        {
            const char *crlf = buf->findCRLF();
            if(crlf)
            {
                ok = ProcessRequestLine(buf->peek() , crlf);
                if(ok)
                {
                    buf->retrieveUntil(crlf + 2);
                    State = HttpRequestPaseState::KExpectHeader;
                }
                else
                {
                    More = false;
                }
            }
            else
            {
                More = false;
            }
        }
        else if(State == HttpRequestPaseState::KExpectHeader)
        {
            const char *crlf = buf->findCRLF();
            if(crlf)
            {
                const char *colon = std::find(buf->peek() , crlf , ':');
                if(colon != crlf)
                {
                    Request_.addHeader(buf->peek() , colon , crlf);
                }
                else
                {
                    State = HttpRequestPaseState::KExpectBody;
                }

                buf->retrieveUntil(crlf + 2);
            }
            else
            {
                More = false;
            }
        }
        else if(State == HttpRequestPaseState::KExpectBody)
        {
            if(buf->readableBytes())
            {
                Request_.SetQuery(buf->peek() , buf->beginWirte());
            }

            State = HttpRequestPaseState::KGotAll;
            More = false;
        }
    }

    return ok;
}

//专门处理HTTP请求的那一行
bool HttpContext::ProcessRequestLine(const char *begin , const char *end)
{
    //请求行有固定格式Method URL Version CRLF，URL中可能带有请求参数 , 例如:GET /index.html HTTP/1.1
	//根据空格符进行分割成三段字符。URL可能带有请求参数，使用"?”分割解析

    bool success = true;
    const char* start = begin;
    const char* space = std::find(start , end , ' ');

    if(space != end && Request_.SetMethod(start , space))
    {
        //继续解析路径和查询参数
        start = space + 1;
        space = std::find(start , end , ' ');       //继续寻找第二个空格
        if(space != end)
        {
            //如果没有到行末，那就证明有其他参数
            const char *Qusetion = std::find(start , space , '?');            //看看有没有其他请求的参数
            if(Qusetion != space)
            {
                //那就证明有请求参数
                //分割请求行为请求参数和Path

                Request_.SetPath(start , Qusetion);
                Request_.SetQuery(Qusetion , space);
            }
            else
            {
                Request_.SetPath(start , space);
            }
        }

        start = space + 1;
        
        //现在来解析HTTP协议的版本
        std::string Version(start , end);
        if(Version == "HTTP/1.0")
        {
            Request_.SetVersion(HttpRequest::Version::Http10);
        }
        else if(Version == "HTTP/1.1")
        {
            Request_.SetVersion(HttpRequest::Version::Http11);
        }
        else 
        {
            success = false;
        }

    }

    return success;
}

//复用HttpContext
void HttpContext::Reset()
{
    State = HttpRequestPaseState::KExpectRequestLine;
    HttpRequest request;
    Request_.swap(request);
}