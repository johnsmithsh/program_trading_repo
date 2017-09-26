#ifndef __MXX_IPC_SHM_H_
#define __MXX_ITC_SHM_H_

/*
 *
 * 共享内存
 *
 *
 * 使用说明:
 *    1.共享内存可在父子进程间共享;
 *      也可在无关联之间的进程间共享;
 *
 *    2.使用共享内存,尽量使用write/read,而不使用标准输入输出(有缓存,也可以删除缓存);??
 *
 *    3.信号处理:SIGSEGV(无效内存引用); centos默认动作: 终止进程;
 *       测试发现,如果两个进程打开同一个共享内存,其中一个进程shm_unlink,则另一个进程会收到SGISEGV信号;
 *    4.同一进程可多次shm_oepn同一共享内存; 每次返回的文件描述符都不一致;
 *      一旦shm_unlink,则共享内存会被删除;
 *
 *    5.共享内存路径
 *       在centos测试,/tmp/test.shm_px做路径进行shm_open,返回错误;
 *                    test.shm_px做路径进行shm_open,则返回成功;共享文件创建路径/dev/shm/test.shm_px
 *                    ./test1.shm_px进行shm_open,则返回成功;  共享文件创建路径/dev/shm/test1.shm_px
 *                   /test2.shm_px进行shm_open,则返回成功;  共享文件创建路径/dev/shm/test2.shm_px
 * 
 * */


#define MXX_IPC_SHM_SUCC                    (0)   //成功                  
#define MXX_IPC_SHM_FAILED                 (-1)   //失败
#define MXX_IPC_SHM_OPEN_FAILED            (-2)   //打开共享内存失败
#define MXX_IPC_SHM_CLOSE_FAILED           (-5)   //关闭共享内存失败:
#define MXX_IPC_SHM_MMAP_FAILED            (-3)   //mmap失败
#define MXX_IPC_SHM_UNMAP_FAILED           (-4)   //unmap失败
#define MXX_IPC_SHM_INIT_FAILED             (-1001) //初始化失败
#define MXX_IPC_SHM_LOCK_FAILED      (-1003) //加锁失败
#define MXX_IPC_SHM_UNLOCK_FAILED    (-1004) //解锁失败

////////////////////////////////////////////////////////////////////////////////
//功能: 打开共享内存
//参数:
//   [in]filepath:路径
//   [in]shm_size:共享内存大小
//返回值:
//    >0文件描述符; <0-失败; 不会等于0;
int mxx_shm_open(const char *filepath, int shm_size);

//功能: 共享内存中分配地址;(即将共享内存映射到该进程内存空间)
//参数: 
//   [in]shm_fd: 共享内存文件描述符;
//   [in]offset: 共享内存的偏移地址(从共享内存的偏移地址开始映射,大小由size指定);
//   [in]size:   映射的共享内存大小;
//返回值:
//   返回映射地址; NULL-失败;
unsigned char * mxx_shm_alloc(int shm_fd, int offset,  int shm_size);

//功能: 释放共享内存分配的地址; (即解除映射)
//参数:
//  [in]shm_addr:映射的地址空间
//  [in]shm_size: 映射地址空间的大小
//返回值:
//   0-成功; ;<0-失败;
int mxx_shm_free(unsigned char *shm_addr,int shm_size);

//功能: 关闭共享内存
//参数:
//  [in]filepath:路径;
//返回值:
//   0-成功; <0-失败;
int mxx_shm_close(const char *shm_filepath);


/////////////////////////////////////////////////////////////////////////////////

//封装共享内存
class IpcShm
{
   public:
       //功能: 构造函数
       IpcShm();
       //功能:析构函数,释放互斥量和条件变量
       virtual ~IpcShm();
      
       //初始化; filepath-文件名,含路径
       int init(const char *filepath, int shm_size);
       
       //功能: 共享内存映射到进程内存地址
       //返回值: 0-成功; <0-失败;
       unsigned char * alloc(int offset, int size); 

       unsigned char * get_addr() { return m_shm_addr; }

       //释放共享内存映射       
       int free();

       //功能: 关闭共享内存
       //返回值: 0-成功; <0-失败;
       int close(); 
       
   private:
      //禁用拷贝构造函数
      IpcShm(const IpcShm &obj);
      //禁用赋值运算符
      IpcShm & operator=(IpcShm &obj);
   private:
     int  m_shmfd;//文件描述符
     char m_filepath[256];
     bool b_init_succ_flag;//true-初始化成功; false-初始化失败;
     unsigned char *m_shm_addr;
     int m_shm_size;
   private:
      //功能: 判断信号量是否初始化成功;
      bool is_init_succ() { return b_init_succ_flag;}
      
};

#endif

