#pragma
#ifndef FIXEDBUFFER_HPP
#define FIXEDBUFFER_HPP

#include <stdio.h>
#include <string.h>
#include <string>

//对于这个日志类我们的核心设计思想就是:每次输出日志的时候算一次存储事件，当存储的日志内存达到一定程度的时候再进行储存的操作

const int KSmallBuffer = 1024;
const int KLargeBuffer = 1024 * 400;

//我们这里来声明一个模板类来存放日志信息

template<int SIZE>
class FixedBuffer
{
private:
    //当前数据尾部的指针和存储数据的地方
    char * cur_;
    char data[SIZE];

    //获取末尾的指针
    const char * End() const {return data + sizeof(data); }
	//模型
	// data_                cur_
	// 	 |   (已存放的数据)  | 剩余的还可存放的大小
	// 	 ——————————————————————————————————————————
	//实际存放的数据

public:
    FixedBuffer() : cur_(data) {}
    
    //往缓冲区添加数据
    void Append(const char *buf , size_t len)
    {
        if(Available() > static_cast<int>(len))
        {
            memcpy(cur_ , buf , len);
            AppendComplete(len);
        }
    }

    //更新指针位置
    void AppendComplete(size_t len) {cur_ += len; }

    //清空数据
    void Menset() { memset(data , 0 , sizeof(data)); }

    //重置数据
    void Reset() { cur_ = data; }

    //获取数据长度
    int Length() const {return static_cast<int>(cur_ - data); }

    //获取数据
    const char *Data() const {return data;}

    //获取当前数据尾部指针
    char * Current() {return cur_;}

    int Available() const{ return static_cast<int>(cur_ - data); }
};



#endif 