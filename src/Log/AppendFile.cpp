#include "AppendFile.hpp"

//fopen:第一个参数:文件名 ， 第二个参数:打开的方式
AppendFile::AppendFile(const std::string& File_Name) : fd_(fopen(File_Name.c_str() , "ae"))
{
    if(fd_ == nullptr)
    {
        printf("log file open failed: errno = %d reason = %s \n", errno, strerror(errno));
    }
    else
    {
        //设置缓冲区
        setbuffer(fd_ , buffer , sizeof(buffer));
    }
}

AppendFile::~AppendFile()
{
    if(fd_)
        fclose(fd_);
}

//真正的添加数据到磁盘文件，调用fwrite()
void AppendFile::Append(const char* logline, size_t len)
{
	//可能数据没法一次完全写入，所以这样做的
	size_t written = 0;
	while (written != len) 
    {
		auto remain = len - written;
		size_t n = fwrite_unlocked(logline+written, 1, remain, fd_);	//这是fwrite()的无锁版本
		if (n != remain) 
        {
			int err = ferror(fd_);
			if (err) 
            {
				fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
				break;
			}
		}
		written += n;
	}
	WrittenBytes += written;

}

//冲刷缓冲区的内容写入到磁盘文件
void AppendFile::Flush()
{
	//功能是冲洗流中的信息，该函数通常用于处理磁盘文件。fflush()会强迫将缓冲区内的数据写回参数stream 指定的文件中。
	fflush(fd_);
}

