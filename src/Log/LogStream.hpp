#pragma
#ifndef LOGSTREAM_HPP
#define LOGSTREAM_HPP

#include "FixedBuffer.hpp"

const int KSmallBuffers = 1024;
const int KLargeBuffers = 1024 * 400;

//日志流相关的类，最主要功能是通过重载<<然后往缓冲区写入消息
//对了，如果<<1的是数字的话，那么还需要转化为字符串


class LogStream
{
public:

    //这是我们自定义的缓冲区
    using Buffer = FixedBuffer<KSmallBuffers>;
    //重载流输出操作符
    LogStream& operator<<(bool v)
    {
        buffer_.Append(v ? "1" : "0" , 1);
        return *this;
    }
    LogStream& operator<<(short);
    LogStream& operator<<(int);
    LogStream& operator<<(long);
    LogStream& operator<<(long long);
    LogStream& operator<<(float);
    LogStream& operator<<(double);
    LogStream& operator<<(char);
    LogStream& operator<<(const char*);
    LogStream& operator<<(const std::string&);

    //把数据输出到缓冲区
    void Append(const char *data , size_t len) {buffer_.Append(data , len); }

    //获取缓冲区对象
    const Buffer& buffer()const { return buffer_; }

private:

    // 格式化数字类型为字符串并输出到缓冲区
    template<class T>
    void FormatInteger(T);

    //这个是我们的缓冲区
    Buffer buffer_;
    //数字转化字符的上限
    const int KMaxNumberSize = 40;
};


#endif