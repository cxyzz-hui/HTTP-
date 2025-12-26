#include "CurrentThread.hpp"
#include <stdio.h>

namespace CurrentThread
{
    thread_local int Cached_Tid = 0;
    thread_local char Tid_String[32];
    thread_local int Tid_String_Lenth = 6;

    pid_t getTid()
    {
        //获取真实的线程ID
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    void CachedTid()
	{
		if (Cached_Tid == 0)
		{
			Cached_Tid = getTid();
			Tid_String_Lenth = snprintf(Tid_String, sizeof(Tid_String), "%5d ", Cached_Tid);
		}
	}
};