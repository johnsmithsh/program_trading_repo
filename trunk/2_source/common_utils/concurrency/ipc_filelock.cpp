#include "ipc_filelock.h"

#include <unistd.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////////

//功能: 封装fcntl调用, 实现加锁解锁功能;
//参数:
//  [in]fd:文件描述符;
//  [in]cmd: F_SETLK/F_SETLKW  F_SETLK-不阻塞; F_SETLKW-阻塞;
//  [in]type:锁类型, F_RDLCK/F_WRLCK/F_UNLCK
//  [in]whence, offset、len:范围
//返回值: -1加锁或解锁失败;
int lock_reg(int fd, int cmd, int type, long offset, int whence, int len)
{
  struct flock lock;
  lock.l_type=type;/*F_RCLCK/F_WRLCK/F_UNLCK*/
  lock.l_start=offset; /*相对于whence的偏移位置*/
  lock.l_whence=whence;/*SEEK_SET,SEEK_CUR,SEEK_END*/
  lock.l_len = len; /*加锁长度*/

  return (fcntl(fd,cmd,&lock)); /* -1表示失败 */
}

//判断是否可进行相应的加锁解锁操作
pid_t lock_test(int fd, int type, long offset, int whence, int len)
{
  struct flock lock;
  
  lock.l_type=type;/*F_RCLCK/F_WRLCK/F_UNLCK*/
  lock.l_start=offset; /*相对于whence的偏移位置*/
  lock.l_whence=whence;/*SEEK_SET,SEEK_CUR,SEEK_END*/
  lock.l_len = len; /*加锁长度*/
  if(fcntl(fd, F_SETLK, &lock)) //if(fcntl(fd,cmd,&lock)==-1); /* -1表示失败 */  
    return -1;

  if(lock.l_type==F_UNLCK)//解锁,直接返回成功即可
    return 0;

  return (lock.l_pid);//
}

/////////////////////////////////////////////////////////////////////////////////////////////

//加锁位置,不影响正常使用即可
#define FILELOCK_POSITION 0x40000000 //加锁位置
#define FILELOCK_MAXLEN  0x3FFFFFFF //加锁长度

#define FLT_LOCK    F_WRLCK  //加锁
#define FLT_UNLOCK  F_UNLCK  //解锁

/*
 * fcntl锁说明:
 *    0. fcntl是posix标准操作;
 *    1. 该锁属于非强制性锁,又名劝告锁(advisory lock);
 *      即进程可以无视另一进程加锁而对文件进行读写操作;
 *    2. 测试发现统一进行可多次加锁,然后一次解锁(即类似于递归锁)
 */


/* 功能:对文件加锁或解锁(全部按照写锁)
 *    注: 该函数不会阻塞进程或线程,立即返回;
 * 参数:
 *  [in]fd:文件描述符;
 *  [in]type: FLT_LOCK-加锁; FLT_UNLOCK-解锁;
 *  [in]block_flag:阻塞标记; 0-不阻塞; 1-阻塞;
 *  [in]offset,len:文件加锁或解锁范围;
 * 返回值:
 *    0-成功; <0-失败;
 */
int fd_lock_unlock(int fd, int type,int block_flag, unsigned int offset, unsigned int len)
{
  struct flock flck;
  flck.l_type=type;
  flck.l_whence=SEEK_SET;
  flck.l_start = offset;
  flck.l_len=len;
  flck.l_pid=getpid();
  if(0==block_flag)//不会阻塞
    return (-1==fcntl(fd, F_SETLK, &flck)) ? -1 : 0;
  else
    return (-1==fcntl(fd, F_SETLKW, &flck)) ? -1 : 0;
}

//构造函数
IpcFileLock::IpcFileLock()
{
  memset(m_filepath, 0, sizeof(m_filepath));
  //m_fp=NULL;
  m_fd=0;
  b_init_succ_flag=false;
}

//功能:析构函数
IpcFileLock::~IpcFileLock()
{
  if(is_init_succ())
  {
    b_init_succ_flag=false;
  }
  //if(NULL!=m_fp)
  //{
  //  fclose(m_fp);
  //  m_fp=NULL;
  //}
  
  if(m_fd>0)
  {
    close(m_fd);
    m_fd=0;
  }
}

int IpcFileLock::init(const char *filepath)
{
  if(is_init_succ()) return MXX_FILELOCK_SUCC;
    
  strncpy(m_filepath, filepath, sizeof(m_filepath));
  
  //fopen打开后需要转换为文件描述符,还不如直接使用open
  //m_fp=fopen(m_filepath, "rw+");
  //if(NULL==m_fp)
  //{
  //  b_init_succ_flag=false;
  //  memset(m_filepath, 0, sizeof(m_filepath));
  //  return IPC_ERROR_INIT_FAILED;
  //}
  
  
  //m_fd=open(m_filepath, O_RDWR, S_IREAD|S_IWRITE);
  //if(m_fd<0)
  //{
  //  printf("Warning: cann't open file!rc=[%d], errno=[%d], strerr=[%s]\n", m_fd, errno, strerror(errno));
  //}
  
  m_fd=open(m_filepath, O_CREAT|O_RDWR, S_IRGRP);//S_IREAD|S_IWRITE);
  if(m_fd<0)
  {
    b_init_succ_flag=false;
    memset(m_filepath, 0, sizeof(m_filepath));
    return MXX_FILELOCK_FAILED;
  }
  
  b_init_succ_flag=true;
  return MXX_FILELOCK_SUCC;
}

//加锁: 阻塞
int IpcFileLock::lock()
{
  if(!is_init_succ())
    return MXX_FILELOCK_INIT_FAILED;
    
  if(0==fd_lock_unlock(m_fd,FLT_LOCK ,1, FILELOCK_POSITION, FILELOCK_MAXLEN))
    return MXX_FILELOCK_SUCC;
  else
    return MXX_FILELOCK_LOCK_FAILED;

}

//加锁:不阻塞
int IpcFileLock::try_lock()
{
  if(!is_init_succ())
    return MXX_FILELOCK_INIT_FAILED;
    
  if(0==fd_lock_unlock(m_fd,FLT_LOCK , 0, FILELOCK_POSITION, FILELOCK_MAXLEN))
    return MXX_FILELOCK_SUCC;
  else
      return MXX_FILELOCK_LOCK_FAILED;

}

//解锁
int IpcFileLock::unlock()
{
  if(!is_init_succ())
    return MXX_FILELOCK_INIT_FAILED;
  
  if(0==fd_lock_unlock(m_fd,FLT_UNLOCK, 0, FILELOCK_POSITION, FILELOCK_MAXLEN))
    return MXX_FILELOCK_SUCC;
  else
    return MXX_FILELOCK_UNLOCK_FAILED;
}

