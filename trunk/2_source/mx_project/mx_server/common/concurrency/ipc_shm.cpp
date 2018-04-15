#include "ipc_shm.h"

#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //posix默认共享内存权限;

/////////////////////////////////////////////////////////////////////////////////////////////
//打开共享内存
int mxx_shm_open(const char *filepath, int shm_size)
{
   int shmfd;
   shmfd=shm_open(filepath, O_RDWR | O_CREAT, FILE_MODE);
   if(shmfd<0)
      return MXX_IPC_SHM_OPEN_FAILED;
   if(shm_size>0)
     ftruncate(shmfd, shm_size);
   return shmfd;
}

//共享内存映射到本进程;
unsigned char * mxx_shm_alloc(int shm_fd, int offset,  int shm_size)
{   
   if(offset<=0) offset=0;
   unsigned char * ptr=(unsigned char *)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, offset);;
   if(MAP_FAILED==(void *)ptr)
      return NULL;
   return ptr;
}

//释放映射内存地址; 0-成功;<0-失败;
int mxx_shm_free(unsigned char *shm_addr,int shm_size)
{
   if(NULL==shm_addr) return -1;
   return munmap(shm_addr, shm_size)<0 ? MXX_IPC_SHM_UNMAP_FAILED : 0 ;
}

//关闭共享内存
int mxx_shm_close(const char *shm_filepath)
{
  if(NULL==shm_filepath) return -1;
  return shm_unlink(shm_filepath)<0 ? -1 : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////


/*
 * 共享内存说明:
 *
 */


//构造函数
IpcShm::IpcShm()
{
   memset(m_filepath, 0, sizeof(m_filepath));
   m_shmfd=0;
   b_init_succ_flag=false;
}

//功能:析构函数
IpcShm::~IpcShm()
{
  if(is_init_succ())
  {
    this->close();
    b_init_succ_flag=false;
  }
  
}

int IpcShm::init(const char *filepath, int size)
{
  if(is_init_succ()) return MXX_IPC_SHM_SUCC;
    
  strncpy(m_filepath, filepath, sizeof(m_filepath));
  
  m_shmfd=mxx_shm_open(m_filepath, size);
  if(m_shmfd<0)
  {
    b_init_succ_flag=false;
    memset(m_filepath, 0, sizeof(m_filepath));
    return MXX_IPC_SHM_INIT_FAILED;
  }
  
  b_init_succ_flag=true;
  return MXX_IPC_SHM_SUCC;
}

//将共享内存映射到内存地址;
unsigned char * IpcShm::alloc(int offset, int size)
{
  if(!is_init_succ())
    return NULL;
  
  //已经申请,则直接返回即可;
  if(NULL!=m_shm_addr) return m_shm_addr;
  
   m_shm_addr=mxx_shm_alloc(m_shmfd, offset, size);
   if(NULL==m_shm_addr) return NULL;
   m_shm_size=size;
   return m_shm_addr;
}

//释放共享内存地址; 0-成功; <0-失败;
int IpcShm::free()
{
  if(!is_init_succ())
    return MXX_IPC_SHM_SUCC;
  if(NULL==m_shm_addr) return MXX_IPC_SHM_SUCC; 
  int rc=mxx_shm_free(m_shm_addr, m_shm_size);  

  if(rc<0) return MXX_IPC_SHM_UNMAP_FAILED;
 
  m_shm_addr=NULL;
  m_shm_size=0;
  return MXX_IPC_SHM_SUCC;
}

//关闭共享内存; <0-失败; 0-成功;
int IpcShm::close()
{
  if(!is_init_succ())
    return MXX_IPC_SHM_SUCC;

  if(NULL!=m_shm_addr)
     free();
  
  int rc=mxx_shm_close(m_filepath);
  if(rc<0)
    return MXX_IPC_SHM_CLOSE_FAILED;

  m_shmfd=0;
  b_init_succ_flag=false;
  return MXX_IPC_SHM_SUCC;
}

