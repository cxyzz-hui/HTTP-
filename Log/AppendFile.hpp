#pragma
#ifndef APPENDFILE_HPP
#define APPENDFILE_HPP

#include <string>
#include <string.h>
#include <stdio.h>
#include <errno.h>

//底层控制磁盘输出的类
class AppendFile
{
private:
    //文件描述符，缓冲区，和往磁盘写入的字节大小
    FILE *fd_;
    char buffer[1024 * 600];
    off_t WrittenBytes;

public:
    explicit AppendFile(const std::string& File_Name);
    ~AppendFile();

    //将数据写入磁盘中
    void Append(const char *data , size_t len);

    //冲刷缓冲区
    void Flush();

    off_t writtenBytes() const {return WrittenBytes; }
};



#endif