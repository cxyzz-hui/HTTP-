#include "AsyncLogger.hpp"
#include <functional>
#include <chrono>
#include <stdio.h>
#include <unistd.h>
#include "LogFile.hpp"

/*
    我们这里的设计思想主要是Append作为与前端交换日志的函数，我们平时写入日志的时候要用Append写在CurrentBuffer上面
    当我们这个日志写满了，那么我们就把它转到NextBuffer上面，然后NextBuffer写入硬盘，CurrentBuffer清空继续接受数据从而完成这一串的循环
*/

AsyncLogger::AsyncLogger(const std::string FileName_ , off_t Roll_Size_ , int Flush_time)
:Flush_Interval(Flush_time)
,BaseName(FileName_)
,Rollsize(Roll_Size_)
,running(false)
,Current_Buffer(std::make_unique<Buffer>())
,Next_Buffer(std::make_unique<Buffer>())
{
    //清空并且扩充容器的容量
    Current_Buffer->Menset();
    Next_Buffer->Menset();
    Buffers.reserve(16);
}

void AsyncLogger::Append(const char *str , int len)
{
    //这里是异步操作并且设计数据删改要加锁
    std::unique_lock<std::mutex>lock(mutex);

    //如果此时剩余内存够的话，直接写入
    if(Current_Buffer->Available() > len)
    {
        Current_Buffer->Append(str , len);
    }
    else
    {
        //没有的话证明这个缓冲区已经满了，把它丢给后端log线程写入文件
        Buffers.emplace_back(std::move(Current_Buffer));
        //这里涉及到了生产者-消费者的设计思想
        //由于我们这个是在多线程的竞态中，我们调用了线程那么会消耗NextBuffer来写入磁盘，如果我们此时没有NextBuffer的话，那么我们就需要重新创建一个新的Buffer
        if(Next_Buffer)
        {
            Current_Buffer = std::move(Next_Buffer);
        }
        else
        {
            //我们使用了move此时我们的CurrentBuffer已经是空指针了，我们这里要重新搞一个Buffer出来
            /*
                void reset(T* new_ptr = nullptr) 
                {
                    T* old_ptr = ptr;  // 保存旧指针
                    ptr = new_ptr;     // 设置新指针
                    delete old_ptr;    // 删除旧对象
                }
            */
           //转移CurrentBuffer指针指针控制的对象
            Current_Buffer.reset(new Buffer);
        }

        //这里正好写满了那么我们召唤一个消费者线程来写入磁盘
        Current_Buffer->Append(str , len);
        cond.notify_one();
    }
}

void AsyncLogger::ThreadFunc()
{
    //创建LogFile文件来打开磁盘文件
    LogFile Out_Put(BaseName , Rollsize);

    //创建临时的变量以便接受前端的数据
    BufferPtr newBuffer1 = std::make_unique<Buffer>();
    BufferPtr newBuffer2 = std::make_unique<Buffer>();

    BufferVector BufferToWrite;
    BufferToWrite.reserve(16);          //预存储16个空间

    while(running)
    {
        //开个栈内存加锁做修改
        //注意这里是数据交换的临界区，做数据交换的时候要多加小心
        {
            //这个获得锁的逻辑是这样的:先获取锁，然后检查是否有相应日志数据，没有的话就释放锁，3秒后再尝试获取锁
            std::unique_lock<std::mutex>lock(mutex);
            if(Buffers.empty())
            {
                //这里会进行堵塞
                cond.wait_for(lock , std::chrono::seconds(Flush_Interval));
            }

            //拷贝资源
            Buffers.push_back(std::move(Current_Buffer));
            Current_Buffer.reset();                         //将对象放入了容器内，所以要滞空
            Current_Buffer = std::move(newBuffer1);
            BufferToWrite.swap(Buffers);

            //如果这个NextBuffer的缓冲区位空指针，那就给他一个Buffer类来控制，保证生产者的顺利执行
            if(!Next_Buffer)
            {
                Next_Buffer = std::move(newBuffer2);
            }
        }


        // 如果缓冲区过多，说明前端产生log的速度远大于后端消费的速度，这里只是简单的将它们丢弃
        if(BufferToWrite.size() > 25)
        {
            BufferToWrite.erase(BufferToWrite.begin() + 2 , BufferToWrite.end());
        }

        //现在将数据拷贝到相应的文件中
        for(size_t i = 0; i < BufferToWrite.size(); i++)
        {
            Out_Put.Append(BufferToWrite[i]->Data() , BufferToWrite[i]->Length());
        }

        //将多余的数据丢掉
        if(BufferToWrite.size() > 2)
        {
            BufferToWrite.resize(2);
        }

        //恢复后端备用的
        if(!newBuffer1)
        {
            newBuffer1 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
            //重置数据
            newBuffer1->Reset();
        }
        if(newBuffer2)
        {
            newBuffer2 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
            newBuffer2->Reset();
        }

        //清空缓冲区
        BufferToWrite.clear();
        Out_Put.Flush();
    }

    Out_Put.Flush();
}