#include "Logger.hpp"
#include "AsyncLogger.hpp"
#include "../Timestamp.hpp"
#include "../CurrentThread.hpp"

#include <thread>
#include <memory>
#include <mutex>
#include <assert.h>

thread_local char t_time[64];             //记录字符串时间的数组
thread_local time_t cur_Seconds;        //当前线程的秒数

static std::unique_ptr<AsyncLogger> Async_Logger;
static std::once_flag global_once_flag;

//初始化日志等级的函数
Logger::LogLevel InitLogLevel()
{
    if(getenv("LGG_DEBUF"))
        return Logger::LogLevel::DEBUG;
    else
        return Logger::LogLevel::INFO;
}

Logger::LogLevel global_LogLevel = InitLogLevel();

Logger::LogLevel Logger::GlobalLogLevel()
{
    return global_LogLevel;
}

void Logger::SetLogLevel(Logger::LogLevel level)
{
	global_LogLevel = level;
}

void DefaultOutput(const char *str , int len)
{
    fwrite(str , 1 , len , stdout);
}

void DefaultFlush()
{
    fflush(stdout);
}

std::string Logger::Log_File_BaseName = "./li22";

void OnceInit()
{
    Async_Logger = std::make_unique<AsyncLogger>(Logger::LogFileName() , 1024 * 1024 * 50);     //50M
    Async_Logger->start();
}

//输出到特定的缓冲区
void Async_Output(const char *logline , int len)
{
    //call_once是为了保证某个函数在多个线程中只执行一次
    std::call_once(global_once_flag , OnceInit);
    printf("AsyncOutput() call_once...\n");
    Async_Logger->Append(logline , len);
}

Logger::OutputFunc Global_Output = Async_Output;
Logger::FlushFunc Global_Flush = DefaultFlush;

//对字符串输出进行优化
class T
{
public:
    T(const char *str_ , unsigned len_) 
    :str(str_)
    ,len(len_)
    {}

    const char *str;
    unsigned len;
};

//这个可以支持链式调用
inline LogStream& operator<<(LogStream&s , T v)
{
    s.Append(v.str , v.len);
    return s;
}

//日志等级字符串数组
const char* global_loglevel_name[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] =
{
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

/*Logger::Logger(const char *FileName_ , int line_ , LogLevel Level_ , const char *funcName)
:Stream_()
,Level(Level_)
,FileName(FileName_)
,line(line_)
{
    //初始化时间
    formatTime();

    //输出日志等级以及函数名等信息
    Stream_ << " " << global_loglevel_name[static_cast<int>(Level)] << funcName << "():";
}


Logger::~Logger()
{
    Stream_ << " - " << FileName << ":" << line << "\n";

    //获取缓冲区的内容并且输出到控制台上
    const LogStream::Buffer& buf(Stream().buffer());
    DefaultOutput(buf.Data() , buf.Length());
}

void Logger::formatTime()
{
    //获取从世纪初到现在的时间
    const char *now = Timestamp::now().toFormattedString().      c_str();
    Stream_ << now;
}
*/

//多种构造函数
Logger::Logger(const char *FileName_ , int line_ , LogLevel Level_ , const char *funcName)
:Impl_(Level_ , FileName_ , line_)
{
    Impl_.Stream_ << funcName << " ";
}
Logger::Logger(const char *FileName , int line)
:Impl_(Logger::LogLevel::INFO , FileName , line)
{

}
Logger::Logger(const char *FileName , int line , LogLevel Level_)
:Impl_(Level_ , FileName , line)
{
    
}
Logger::Logger(const char *FileName , int line , bool ToAbort)
:Impl_(ToAbort ? LogLevel::FATAL : LogLevel::ERROR , FileName , line)
{
    
}

Logger::~Logger()
{
    Impl_.finish();

    //获取缓冲区的内容并且输出到控制台上
    const LogStream::Buffer& buf(Stream().buffer());
    DefaultOutput(buf.Data() , buf.Length());
}

Logger::Impl::Impl(LogLevel level , const std::string& File , int line)
:Time(Timestamp::now())
,Stream_()
,Level(level)
,Line(line)
,FileName(File.c_str())
{
    formatTime();               //时间输出
    CurrentThread::tid();       //更新线程
    Stream_ << T(CurrentThread::TidString(), CurrentThread::TidStringLenth());       //输出线程id
    Stream_ << T(global_loglevel_name[static_cast<int>(Level)] , 6);
}

void Logger::Impl::formatTime()
{
    //先获取秒和微秒
    int64_t mircoSecondSinceEpoch = Time.microSecondSinceEpoch();
    time_t Seconds = static_cast<time_t>(mircoSecondSinceEpoch / Time.KMicroSecondPerSecond);
    int microSeconds = static_cast<time_t>(mircoSecondSinceEpoch % Time.KMicroSecondPerSecond);

    //秒数发生变化的时候进行格式化
    if(microSeconds != cur_Seconds)
    {
        cur_Seconds = microSeconds;
    
        //转化为本地时间
        struct tm tm_time;
        memset(&tm_time , 0 , sizeof(tm_time));
        //线程安全版本的:将 UTC 时间戳转换为本地时间
        localtime_r(&Seconds , &tm_time);

          // 格式化为：YYYYMMDD HH:MM:SS
        int len = snprintf(t_time, sizeof(t_time), "%4d%0202d %02d:%02d:%02d",
            tm_time.tm_year + 1900,  // 年（从1900开始）
            tm_time.tm_mon + 1,      // 月（0-11 → 1-12）
            tm_time.tm_mday,         // 日
            tm_time.tm_hour,         // 时
            tm_time.tm_min,          // 分
            tm_time.tm_sec);         // 秒

        //确保长度是固定的
        assert(len == 17);
    }

    char buf[12] = { 0 };
    int lenMirco = sprintf(buf , ".%06d" , microSeconds);
    Stream_ << T(t_time , 17) << T(buf , lenMirco);
}

void Logger::Impl::finish()
{
    Stream_ << " - " << FileName << ":" << Line << "\n";
}

//设置冲刷和输出的函数
void Logger::SetOutput(OutputFunc out)
{
    Global_Output = out;
}

void Logger::SetFlush(FlushFunc flush)
{
    Global_Flush = flush;
}