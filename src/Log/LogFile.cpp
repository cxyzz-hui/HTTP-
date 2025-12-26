#include "LogFile.hpp"
#include "../util.hpp"
#include <string>

/*LogFile::LogFile(const std::string FileName_ , int Flush_count_)
:FileName(FileName_)
,Flush_count(Flush_count_)
,count(0)
{
    File = std::make_unique<AppendFile>(FileName_);
}*/

LogFile::LogFile(const std::string& BaseName_ , off_t rollSize_ , int FlushInterval , int Flush_count_)
:basename(BaseName_)
,rollSize(rollSize_)
,Flush_Interval_seconds(FlushInterval)
,Flush_count(Flush_count_)
,count(0)
,start_of_period(0)
,last_roll(0)
,last_flush(0)
{
    rollFile();
}

void LogFile::Append(const char *str , int len)
{
    File->Append(str , len);
    //通过文件大小看是否需要滚动日志文件
    if(File->writtenBytes() > rollSize)
    {
        rollFile();
    }
    else
    {
        //这里是意思是通过时间间隔或者时间周期和计数来判断是否需要滚动创建新的文件
        ++count;
        if(count > Flush_count)
        {
            time_t now = ::time(nullptr);
            //转换成当天凌晨0点对应的秒数:举个例子：假设一天一共24秒，从第1天开始计算，那第1天0秒对应的秒数就是0，第2天0秒对应的秒数就是24         
            time_t this_time = now / kRollPerSeconds_ * kRollPerSeconds_;
            if(this_time > start_of_period)
            {
                rollFile();
            }
            else if(now - last_roll > Flush_Interval_seconds)
            {
                last_flush = now;
                File->Flush();
            }
        }
    }

}

//获取日志文件名
std::string LogFile::getLogName(const std::string& BaseName , time_t *now)
{
    std::string filename;
    filename.reserve(BaseName.size() + 64);
    filename = basename;
    char time_buf[64];

    //添加时间戳
    struct tm tm_result;
    memset(&tm_result , 0 , sizeof(tm_result));
    localtime_r(now,&tm_result);                      //转化为UTF-8的本地时间
    strftime(time_buf, sizeof(time_buf), ".%Y%m%d-%H%M%S.", &tm_result);        //将时间结构体格式化指定字符串
    
    filename += time_buf;

    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf) , "%d.log" , ProcessInfo::pid());
    filename += pidbuf;

    printf("getLogFileName()...\n");
    return filename;
}   

/*
void LogFile::Append(const char *str , int len)
{
    //写入文件
    File->Append(str , len);
    if(++count >= Flush_count)
    {
        count = 0;
        File->Flush();
    }
}
*/

//主要功能创建一个新的日志文件
bool LogFile::rollFile()
{
    time_t now = time(nullptr);

    //进行条件检查避免多次滑动
    if(now > last_roll)
    {
        //流程:生成新文件->更新时间->计算周期->创建新文件
        std::string fileName = getLogName(basename , &now);

        //记录当前时间
        last_flush = now;
        last_roll = now;

        //我们创建了一个新的文件，那么意味着我们需要重新记录当前时间的起点
        start_of_period = now / kRollPerSeconds_ * kRollPerSeconds_;

        File.reset(new AppendFile(fileName));
        return true;
    }

    return false;
}