#pragma
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "LogStream.hpp"
#include "../Timestamp.hpp"

//这个类负责生产日志
class Logger
{
public:
    //创建一个枚举类累枚举日志的级别
    enum class LogLevel
    {
        DEBUG,      //调试使用
        INFO,       //信息
        WARN,       //警告
        ERROR,      //错误
        FATAL,      //致命错误
        NUM_LOG_LEVELS,
    };

    //void(*)是函数指针的意思
    using OutputFunc = void (*)(const char *str , int len);
    using FlushFunc = void (*) ();

    //输出在哪个函数的哪一行的报错日志
    Logger(const char *FileName_ , int line_ , LogLevel Level_ , const char *funcName);
    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, bool toAbort);

    ~Logger();

    LogStream& Stream() {return Impl_.Stream_; }
    //获取全局的日志等级而非当前的日志等级
    static LogLevel GlobalLogLevel();

    //默认输出函数，直接输出到缓冲区
    void DefaultOutput(const char *msg , int len) { fwrite(msg , 1 , len , stdout); }      

    static void SetOutput(OutputFunc);          // 设置输出函数
    static void SetFlush(FlushFunc);            // 设置冲刷函数
    static void SetLogLevel(Logger::LogLevel level);

    static std::string LogFileName() {return Log_File_BaseName; }

private:

    //由于我们想象中的日志是根据等级不同输出不同类型的字段个数，那么则意味着我们有很多的构造函数
    //那么为了提高代码复用，我们这里再声明一个类把我们相应的私有变量都放在这个类里面，这样就可以提高代码复用率了
    class Impl
    {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level , const std::string& File , int line);
        void finish(); 

        //格式化时间 "%4d%2d%2d %2d:%2d:%2d",并把时间通过strea_<<写入到日志流对象内的缓冲
        void formatTime();
    
        Timestamp Time;
        LogStream Stream_;       //日志流，使用这个的话可以用<<来记录日志
        LogLevel Level;         //日志等级
        const char *FileName;   //使用这个函数的文件名，也就是__FILE__
        int Line;               //行数

        static std::string logFileName_;
    };

    Impl Impl_;
    static std::string Log_File_BaseName;
};


//这里是不同的日志等级就会有对应的输出，有些等级会输出多些字段，有些等级的输出字段会少些
// 宏定义，在代码中include该头文件即可使用： LOG_INFO << "输出数据";
#define LOG_TRACE if (Logger::GlobalLogLevel() <= Logger::LogLevel::TRACE) Logger(__FILE__, __LINE__, Logger::LogLevel::TRACE, __func__).Stream()
#define LOG_DEBUG if (Logger::GlobalLogLevel() <= Logger::LogLevel::DEBUG) Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG, __func__).Stream()
#define LOG_INFO if (Logger::GlobalLogLevel() <= Logger::LogLevel::INFO) Logger(__FILE__, __LINE__).Stream()

#define LOG_WARN Logger(__FILE__, __LINE__, Logger::LogLevel::WARN).Stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR).Stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL).Stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).Stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).Stream()
#endif