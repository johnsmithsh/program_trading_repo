#ifndef __MXX_IPC_SHM_SYSV_H_
#define __MXX_IPC_SHM_SYSV_H_

/**
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
 **/


#define MXX_IPC_SHM_SUCC                    (0)   //成功                  
#define MXX_IPC_SHM_FAILED                 (-1)   //失败
#define MXX_IPC_SHM_OPEN_FAILED            (-2)   //打开共享内存失败
#define MXX_IPC_SHM_CLOSE_FAILED           (-5)   //关闭共享内存失败:
#define MXX_IPC_SHM_MMAP_FAILED            (-3)   //mmap失败
#define MXX_IPC_SHM_UNMAP_FAILED           (-4)   //unmap失败
#define MXX_IPC_SHM_KEY_ID_FAILED          (-5)   //system V ipc对象的key_id错误
#define MXX_IPC_SHM_INIT_FAILED             (-1001) //初始化失败
#define MXX_IPC_SHM_LOCK_FAILED      (-1003) //加锁失败
#define MXX_IPC_SHM_UNLOCK_FAILED    (-1004) //解锁失败

#define MXX_OPENFILE_FAILED   (-1006)


#define MXX_INVALID_SHMID (-1)     //!< 无效shmid
#define MXX_INVALID_FD    (-1) //!< 无效文件描述符
////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////

//封装共享内存
class IpcShmSysV
{
   public:
       //@brief 构造函数
       IpcShmSysV();
       //@brief 析构函数,释放互斥量和条件变量
       virtual ~IpcShmSysV();
      
       /**
        * @brief  打开共享内存
        * @param 
        *    [in]filepath: 文件名,含路径;
        *    [in]shmkey: 共享内存key; <0,则根据文件inode生成
        *    [in]shm_size: 共享内存大小;
        * @retval
        *    0-成功; <0-失败;
        * @note
        *    每个对象open只能调用一次; 多次调用导致共享内存关闭后重新打开;
        **/
       int shm_open(const char *filepath, int shmkey, size_t shm_size);
       
       /**
        * @brief 关闭共享内存
        * @param 无
        * @retval 
        *    0-成功; <0-失败;
        **/
       int shm_close();
       
       /**
        * @brief 共享内存映射到进程空间
        * @param 无
        * @retval 
        *    NULL-失败; 其他-共享内存映射地址
        **/
       unsigned char * mmap();
        
       /**
        * @brief 解除共享内存映射
        * @param 无
        * @retval 
        *    0-成功; <0-失败;
        **/
       int unmap();
       
    
       inline int get_shmkey()   { return m_shmkey; }
       inline int get_shmid()    { return m_shmid; }
       // @brief 获取共享内存映射地址  NULL-共享内存未映射到当前进程空间;    
       inline unsigned char * get_shmaddr() { return (unsigned char *)m_shm_addr; }  
       // @brief 获取共享内存空间大小
       inline int get_shmsize() { return m_shmsize; }
       inline bool is_owner()   { return (m_bCreated==true);}
       // @brief 获取共享内存关联计数
       int get_shm_nattch();
       
   protected://辅助函数
      //不要想太多,发现子函数越多控制越浮渣,考虑的情况越多
      //int shm_open();
      //int shm_close();
      //
      //unsigned char * shm_mmap();
      //int shm_unmap();
      // 
      //
      
      /**
       * @brief 关闭文件
       * @param
       *    [in]remove_flag: true-删除磁盘shm文件; false-仅关闭文件,不删除磁盘文件
       * @retval true-成功; false-失败;
       **/
      bool file_close(bool remove_flag=false);
      
      /**
       * @brief 清空成员变量 unmap->shmctl(IPC_RMID)->关闭文件;
       * @retval 0-成功; <0-失败;
       **/
      int clear_info();
   private:
      //@brief 禁用拷贝构造函数
      IpcShmSysV(const IpcShmSysV &obj);
      //@brief 禁用赋值运算符
      IpcShmSysV & operator=(IpcShmSysV &obj);
   private:
     
     int m_shmkey;//!< 全局可识别key; 调用者传入 or 根据文件inode生成; @see to_shmkey函数
     int  m_fd;//!< 文件描述符, 两个作用: 1.产生shmkey; 2. 提供文件锁
     char m_filepath[256];//!< 文件名用于生成shmkey
     
     int m_shmid;//!< 操作系统共享内存id(system v); 该值有效,则说明shmat映射到当前进程,必须shmctl(RMID)关闭(即计数器递减1);
     void *m_shm_addr; //!< 共享内存映射到该进程指针; 
     int m_shmsize;//!< 共享内存大小,单位:字节; 此处是否根据函数shmctl(IPC_STAT)获取更好; 考虑到速度问题,还是保留参数吧!
     
     bool m_bCreated;      //!< true-说明shm由该实例创建; 否则,由其他对象创建;
     bool b_init_succ_flag;//!< true-初始化成功; false-初始化失败;
   private:
      //功能: 判断信号量是否初始化成功;
      bool is_init_succ() { return b_init_succ_flag;}
      //int to_shmkey();
      
};

#endif

