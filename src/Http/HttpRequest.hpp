#pragma
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <string.h>
#include <unordered_map>

#include "../Time/Timestamp.hpp"

class HttpRequest
{
public:
    HttpRequest(/* args */);
    enum class Method
    {
        KInvalid,
        KGet,
        KPost,
        KHead,
        KPut,
        KDelete
    };

    enum class Version
    {
        UnKnown,
        Http10,
        Http11
    };

    //一些工具函数
    bool SetMethod(const char *statr , const char *end);
    Method GetMethod() {return method;}

    void SetVersion(Version v) {version = v; }
    Version GetVersion() const {return version; }

    const char *methodString() const;

    void SetPath(const char *start , const char *end) {path.assign(start , end); }
    const std::string GetPath() const {return path; }

    void SetQuery(const char *start , const char *end) {query.assign(start , end); }
    const std::string GetQuery() {return query; }

	void SetRecieveTime(Timestamp t) { receiveTime = t; }
	Timestamp ReceiveTime()const { return receiveTime; }

    void addHeader(const char *start , const char * colon , const char *end);
    std::string GetHeader(const std::string failed) const;

    void swap(HttpRequest& that);

    const std::unordered_map<std::string , std::string>& GetHeads() {return Heads; }
private:
    Method method;
    Version version;

    std::string path;           //请求路径
    std::string query;          //请求体

    Timestamp receiveTime;

    std::unordered_map<std::string , std::string>Heads;
};



#endif