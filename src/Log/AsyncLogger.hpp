#pragma
#ifndef ASYNCLOGGER_HPP
#define ASYNCLOGGER_HPP

#include <mutex>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <condition_variable>

#include "FixedBuffer.hpp"


//异步日志，单独开一个线程来记录日志
class AsyncLogger
{
public:
    //参数为日志文件名并且每隔三秒刷新一下缓冲区
    AsyncLogger(const std::string FileName , off_t RollSize_ , int Flush_time = 3);
    ~AsyncLogger()
    {
        if(running)
        {
            stop();
        }

    }

    void Append(const char* LogLine , int len);

    void start()
    {
        running = true;
        Thread = std::thread([this]() {ThreadFunc(); });
    }

    void stop()
    {
        running = false;
        cond.notify_one();     // 停止时notify以便将残留在缓存区中的数据输出
        if(Thread.joinable())
        {
            Thread.join();
        }
    }

public:

    //后端日志线程函数
    void ThreadFunc();

    //声明固定大小的缓冲区
    using Buffer = FixedBuffer<KLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;

    //冲刷间隔
    const int Flush_Interval;
    bool running;
    std::string BaseName;           //文件名

    const off_t Rollsize;

    //线程以及异步相关的东西
    std::thread Thread;             //Log代表的相应线程
    std::mutex mutex;
    std::condition_variable cond;

    //当前缓冲区和下一个缓冲区的相应指针
    BufferPtr Current_Buffer;
    BufferPtr Next_Buffer;

    BufferVector Buffers;           //准备交付给后端log线程使用的缓冲区vector
};


#endif