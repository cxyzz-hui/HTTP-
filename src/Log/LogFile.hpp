#pragma
#ifndef LOGFILE_HPP
#define LOGFILE_HPP

//将日志写入文件的类
#include <string>
#include <memory>
#include "AppendFile.hpp"

class LogFile
{
private:
    const std::string FileName;         //记录日志文件的文件名
    const int Flush_count;              //最多记录这个次数日志就强制冲刷到缓冲区并记录到文件中
    int count;                          //记录调用了多少地刺Append        
    std::unique_ptr<AppendFile>File;    //操作的文件类
    const std::string basename;         //滚动日志文件的基名
    const off_t rollSize;               //滚动日志文件的大小
    const int Flush_Interval_seconds;   //冲刷缓冲区的时间间隔

    //这是与时间有关的相应变量
    time_t start_of_period;              //日志文件开始的时间
    time_t last_roll;                   //最新一次滚动日志的事件
    time_t last_flush;                  //最新一次冲刷缓冲区的时间
    const static int kRollPerSeconds_ = 60 * 60 * 24;	//一天中的秒数

    std::string getLogName(const std::string& baseName , time_t *now);
public:


    LogFile(const std::string& BaseName_ , off_t rollSize_ , int FlushInterval = 3 , int Flush_count_ = 1024);

    bool rollFile();            //滚动日志的函数
    //添加日志到File_的缓冲区
    void Append(const char *str , int len);
    //冲刷缓冲区
    void Flush() { File->Flush(); }
 
};

#endif