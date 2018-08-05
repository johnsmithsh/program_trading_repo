#ifndef __MXX_IPC_FILELOCK_H_
#define __MXX_ITC_FILELOCK_H_

/*
 *
 * 通过fcntl对文件加锁实现一个可以在进程间使用的锁;
 *
 * 注:
 *    fcntl锁是一个劝告锁,不是强制锁; 即进程可以忽略另一个进程的锁而对文件进行读写
 *    fcntl锁要么阻塞要么非阻塞,无法设置超时时间;
 *    fcntl锁，通过一个进程可对同一个字节多次重复加锁,然后解锁一次就可以了;
 *
 * 使用说明:
 * 
 * */

#include <fcntl.h>

#define MXX_FILELOCK_SUCC                    (0)   //成功                  
#define MXX_FILELOCK_FAILED                 (-1)   //没有收到条件变量
#define MXX_FILELOCK_INIT_FAILED             (-1001) //初始化失败
#define MXX_FILELOCK_LOCK_FAILED      (-1003) //加锁失败
#define MXX_FILELOCK_UNLOCK_FAILED    (-1004) //解锁失败

////////////////////////////////////////////////////////////////////////////////
//为简化fcntl文件锁的使用,对其进行封装《unix网络编程:进程间通信》

//功能: 封装fcntl调用, 实现加锁解锁功能;
//参数:
//  [in]fd:文件描述符;
//  [in]cmd: F_SETLK/F_SETLKW  F_SETLK-不阻塞; F_SETLKW-阻塞;
//  [in]type:锁类型, F_RDLCK/F_WRLCK/F_UNLCK
//  [in]whence, offset、len:范围;
//      whence:SEEK_SET/SEEK_END/SEEK_CUR
//返回值: -1加锁或解锁失败;
int lock_reg(int fd, int cmd, int type, long offset, int whence, int len);

//判断是否已经加锁; 0-未加锁; 否则返回加锁进程ID;
pid_t log_test(int fd, int type, long offset, int whence, int len);

#define read_lock(fd, offset, whence, len)  lock_reg(fd, F_SETLK, F_RDLCK,offset,whence,len)
#define readw_lock(fd, offset, whence, len) lock_reg(fd, F_SETLKW,F_RDLCK,offset,whence, len) //阻塞

#define write_lock(fd, offset,whence,len)   lock_reg(fd, F_SETLK, F_WRLCK,offset,whence,len)
#define writew_lock(fd,offset,whence,len)   lock_reg(fd, F_SETLKW,F_WRLCK,offset,whence,len) //阻塞

#define un_lock(fd,offset,whence,len)       lock_reg(fd, F_SETLK, F_UNLCK,offset,whence,len)

//判断是否可进行加锁
#define is_read_lockable(fd,offset,whence,len)  (!lock_test(fd,F_RDLCK,offset,whence,len))
#define is_write_lockable(fd,offset,whence,len) (!lock_test(fd,F_WRLCK,offset,whence,len))


/////////////////////////////////////////////////////////////////////////////////

//封装文件锁
class IpcFileLock
{
   public:
       //功能: 构造函数
       IpcFileLock();
       //功能:析构函数,释放互斥量和条件变量
       virtual ~IpcFileLock();
      
       //初始化; filepath-文件名,含路径
       int init(const char *filepath);
       
       //功能: 加锁; 阻塞
       //返回值: 0-成功; <0-失败;
       int lock(); 
       
       //功能: 加锁; 阻塞
       //返回值: 0-成功; <0-失败;
       int try_lock(); 
       
       //功能: 解锁;
       //返回值: 0-成功; <0-失败;
       int unlock();

   private:
      //禁用拷贝构造函数
      IpcFileLock(const IpcFileLock &obj);
      //禁用赋值运算符
      IpcFileLock & operator=(IpcFileLock &obj);
   private:
     int  m_fd;//文件描述符
     char m_filepath[256];
     bool b_init_succ_flag;//true-初始化成功; false-初始化失败;
   private:
      //功能: 判断信号量是否初始化成功;
      bool is_init_succ() { return b_init_succ_flag;}
      
};

#endif

