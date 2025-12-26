#pragma
#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <queue>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <memory>
#include <thread>

class ThreadPool
{
public:
    using Task = std::function<void()>;
    explicit ThreadPool();
    ~ThreadPool();

    void start(int numThreads);
    void stop();

    void add(Task f);
private:
    void runInThread();

    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<Task>tasks_;
    std::vector<std::unique_ptr<std::thread>>threads_;

    bool running_;
};


#endif