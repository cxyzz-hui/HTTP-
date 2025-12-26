#pragma
#ifndef CURRENTTHREAD_HPP
#define CURRENTTHREAD_HPP

#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    //extern:告诉编译器，这个变量在其他文件
    extern thread_local int Cached_Tid;
    extern thread_local char Tid_String[32];
    extern thread_local int Tid_String_Lenth;

    //获得进程ID
    inline pid_t getTid();

    void CachedTid();

    // 高效获取线程ID
    inline int tid()
    {
        if(__builtin_expect(Cached_Tid == 0 , 0))
        {
            CachedTid();
        }
        return Cached_Tid;
    }

    inline const char *TidString()
    {
        return Tid_String;
    }
    inline int TidStringLenth()
    {
        return Tid_String_Lenth;
    }
};

#endif