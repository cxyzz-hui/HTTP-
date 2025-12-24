#include "LogStream.hpp"
#include <algorithm>
#include <string>
#include <string.h>
#include <stdint.h>

// 高效的整型数字转字符算法, by Matthew Wilson
template<typename T>
size_t Convert(char buf[], T value)
{
    static const char digits[] = "9876543210123456789";
    static const char* zero = digits + 9;
    T i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

//格式化数字类型为字符串并且输出到缓冲区
template<typename T>
void LogStream::FormatInteger(T v)
{
    //看看剩余空间是否还有剩下的，如果有就继续，不然就不继续
    if(buffer_.Available() > KMaxNumberSize)
    {
        //写入缓区
        size_t len = Convert(buffer_.Current() , v);
        buffer_.AppendComplete(len);
    }
}

//这里直接使用强转是为了避免隐性转化
//*this是为了接触引用直接使用这个类本体
LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    // 格式化整型为字符串并输出到缓冲区
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(float v)
{
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(double v)
{
    //直接输出到缓冲区这个double类型
    if(buffer_.Available() > KMaxNumberSize)
    {
        //写入缓区
        int len = snprintf(buffer_.Current() , KMaxNumberSize , "%.12g" , v);
        buffer_.AppendComplete(len);
    }

    return *this;
}

LogStream& LogStream::operator<<(char v)
{
    //直接输出到缓冲区
    buffer_.Append(&v , 1);
    return *this;
}

LogStream& LogStream::operator<<(const char * v)
{
    // 直接输出到缓冲区
    if (v)
    {
        buffer_.Append(v, strlen(v));
    }
    else
    {
        buffer_.Append("(null)", 6);
    }
    return *this;
}

LogStream& LogStream::operator<<(const std::string& v)
{
    buffer_.Append(v.c_str() , v.size());
    return *this;
}