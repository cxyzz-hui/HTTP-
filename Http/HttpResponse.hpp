#pragma
#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <unordered_map>
#include <string>

class Buffer;

class HttpResponse
{
public:

    enum class HttpStateCode
    {
        KUnknown,
        K2000k = 200,
        K301MovedPermanently = 301,
        K400BadRequest = 400,
        K404NotFound = 404
    };

    explicit HttpResponse(bool state);
    void SetStateCode(HttpStateCode code) { StateCode = code; }
    void SetStatusMessage(const std::string Message) { StatusMessage = Message; }
    void SetCloseConnetion(bool on) {CloseConnection = on; }
    bool CloseConnetion_() const {return CloseConnection; }

    void SetContentType(const std::string& ContentType) {}

    void addHeader(const std::string& key , const std::string& value) { Header[key] = value; }

    void SetBoby(const std::string Boby) { boby = Boby; }

    void appendToBuffer(Buffer* output) const;
private:

    std::unordered_map<std::string , std::string>Header;
    HttpStateCode StateCode;
    std::string StatusMessage;
    bool CloseConnection;
    std::string boby;

};

#endif