#include "ThreadPool.hpp"

ThreadPool::ThreadPool() : running_(true) {}

ThreadPool::~ThreadPool()
{
    if(running_)
    {
        running_ = false;
    }
}

void ThreadPool::start(int numThreads)
{
    //预分配空间
    threads_.reserve(numThreads);
    running_ = true;

    //创建线程
    for(int i = 0; i < numThreads; i++)
    {
        //创建unique_ptr对象
        threads_.emplace_back(std::make_unique<std::thread>([this](){
            //printf("thread start\n");
            runInThread();
        }));
    }
}

void ThreadPool::stop()
{
    //对线程进行回收
    {
        //获取锁
        std::unique_lock<std::mutex>lock(mutex_);
        running_ = false;
        cond_.notify_all();
        printf("threadPool is stop\n");
    }

    for(auto& thread_ : threads_)
    {
        //阻塞回收
        thread_->join();
    }
}

//添加任务
void ThreadPool::add(Task task)
{
    //如果线程是空的，直接执行
    if(threads_.empty())
    {
        task();
    }
    else
    {
        {
            std::unique_lock<std::mutex>lock(mutex_);
            if(!running_) return;
            //将任务放入队列
            tasks_.push(std::move(task));
        }

        cond_.notify_one();
    }
}

//执行线程
void ThreadPool::runInThread()
{
    //printf("thread is runing\n");

    //循环处理事件
    while(running_)
    {
        Task task;
        {
            std::unique_lock<std::mutex>lock(mutex_);

            //等待条件成立
            cond_.wait(lock , [this](){return !running_ || !tasks_.empty();});
        }
        if(!tasks_.empty())
        {
            //取出任务来执行
            task = tasks_.front();
            tasks_.pop();
        }
        if(task)
        {
            task();
        }
    }
    
    //printf("thread exit\n");
}