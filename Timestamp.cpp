#include "Timestamp.hpp"
#include <chrono>
#include <inttypes.h>

static_assert(sizeof(Timestamp) == sizeof(int64_t),"Timestamp should be same size as int64_t");

std::string Timestamp::toString() const
{
    char buf[32] = {0};
    //获取准确时间
    int64_t seconds = micro_seconds_since_epoch / KMicroSecondPerSecond;
    int64_t mircoseconds = micro_seconds_since_epoch % KMicroSecondPerSecond;

    //这里用PRId64是为了跨平台的兼容
    snprintf(buf , sizeof(buf) , "%" PRId64 ".06%" PRId64 "", seconds , mircoseconds);

    return buf;
}

//将时间戳转化为可读的事件类型
std::string Timestamp::toFormattedString(bool ShowMicroSeconds) const
{
    char buf[64] = { 0 };

    //将时间转化为UTF类型,便于输出和统一时间
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch / KMicroSecondPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds , &tm_time);

    if(ShowMicroSeconds)
    {
        int microSeconds = static_cast<int>(micro_seconds_since_epoch % KMicroSecondPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
            microSeconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }

    return buf;
}

Timestamp Timestamp::now()
{
    //这个语句是意思是:获取当前时间并转化为微秒并获取从1970-1-1到现在的时间
    auto counts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
   return Timestamp(counts); 
}