#include "Timer.hpp"

std::atomic_int64_t Timer::num_create_;

void Timer::restart(Timestamp now)
{
    if(repeat_)
    {
        expiration_ = addTime(now , interval_);
    }
    else
    {
        expiration_ = Timestamp::invaild();
    }
}