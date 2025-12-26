#pragma
#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

#include <cstdio>
#include <cstdint>
#include <string>

class Timestamp
{
private:
    int64_t micro_seconds_since_epoch;  //表示微秒的变量
public:
    Timestamp() : micro_seconds_since_epoch(0) {}

    explicit Timestamp(int64_t microSecondSinceEpoch) : micro_seconds_since_epoch(microSecondSinceEpoch) {}
    static Timestamp now();
    static Timestamp invaild() {return Timestamp(); };
    std::string toString() const;
    //将日期信息封装为19xx-xx-x的形态
    std::string toFormattedString(bool ShowMicroSecond = true) const;       //格式化字符串
    bool valid() const {return micro_seconds_since_epoch > 0; }         //判断是否有效
    int64_t microSecondSinceEpoch() const {return micro_seconds_since_epoch;}

    static const int KMicroSecondPerSecond = 1000 * 1000;

};

inline bool operator<(Timestamp a , Timestamp b)
{
    return a.microSecondSinceEpoch() < b.microSecondSinceEpoch();
}

inline bool operator==(Timestamp a , Timestamp b)
{
    return a.microSecondSinceEpoch() == b.microSecondSinceEpoch();
}

inline double TimeDifferce(Timestamp high , Timestamp low)
{
    int64_t diff = high.microSecondSinceEpoch() - low.microSecondSinceEpoch();
    return static_cast<double>(diff) / Timestamp::KMicroSecondPerSecond;
}

inline Timestamp addTime(Timestamp timestamp , double seconds)
{
    int64_t extra_time = static_cast<int64_t>(seconds * Timestamp::KMicroSecondPerSecond);
    return Timestamp(timestamp.microSecondSinceEpoch() + extra_time);
}


#endif