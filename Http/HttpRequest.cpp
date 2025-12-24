#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
:method(HttpRequest::Method::KInvalid)
,version(HttpRequest::Version::UnKnown)
{

}

bool HttpRequest::SetMethod(const char *start , const char *end)
{
    std::string text(start , end);
    if(text == "GET")
    {
        method = Method::KGet;
    }
    else if(text == "HEAD")
    {
        method = Method::KHead;
    }
    else if(text == "POST")
    {
        method = Method::KPost;
    }
    else if(text == "PUT")
    {
        method = Method::KPut;
    }
    else if(text == "DELETE")
    {
        method = Method::KDelete;
    }
    else
    {
        method = Method::KInvalid;
    }

    return method != Method::KInvalid;
}

const char* HttpRequest::methodString() const
{
    const char *result = "UNKNOWN";
    switch (method)
    {
    case Method::KGet:
        result = "GET";
        break;
    case Method::KHead:
        result = "HEAD";
        break;
    case Method::KPost:
        result = "POST";
        break;
    case Method::KDelete:
        result = "DELETE";
        break;
    default:
        break;
    }

    return result;
}

void HttpRequest::addHeader(const char *start , const char *colon , const char *end)
{
    //isspace(int c)函数判断字符c是否为空白符
    //说明：当c为空白符时，返回非零值，否则返回零。（空白符指空格、水平制表、垂直制表、换页、回车和换行符。
    // 要求冒号前无空格

    std::string field(start , colon);
    while(colon < end && isspace(*colon)) colon++;

    std::string value(colon , end);
    while (!value.empty() && isspace(value[value.size() - 1])) value.resize(value.size() - 1);

    Heads[field] = value;
}

std::string HttpRequest::GetHeader(const std::string Field) const
{
    std::string result;

    auto it = Heads.find(Field);
    if(it != Heads.end())
    {
        return it->second;
    }

    return result;
}

void HttpRequest::swap(HttpRequest& Request)
{
    std::swap(method , Request.method);
    std::swap(version , Request.version);

    path.swap(Request.path);
    query.swap(Request.query);

    Heads.swap(Request.Heads);
}