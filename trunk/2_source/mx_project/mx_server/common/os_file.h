#ifndef __OS_FILE_H__
#define __OS_FILE_H__
/**
 *
 *
 **/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//功能: 文件大小,单位:字节
//注: 目前仅支持linux,不支持window
inline int os_filesize(const char *filename)
{
    struct stat statbuf;
    memset(&statbuf, 0, sizeof(statbuf));
    
    stat(filename, &statbuf);
    return statbuf.st_size;   
}




#endif
