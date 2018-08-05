
#include <string.h>
#include <stdio.h>
#include <errno.h>
//#include <sys/mman.h>
#ifdef WIN32
#else
  #include <unistd.h>
  #include <sys/ipc.h>
  #include <sys/shm.h>
  #include <sys/stat.h>        /* For mode constants */
  #include <sys/types.h>        /* For mode constants */
  #include <fcntl.h>           /* For O_* constants */
#endif
#include "ipc_shm_sysv.h"

#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //posix默认共享内存权限;
#define SHMID_SUBMASK  16 //!< system v 共享内存id的子id
#define SHM_FLAG 0 //!< 共享内存对象shmflag, @see shmat参数shmflag
/////////////////////////////////////////////////////////////////////////////////////////////
//打开共享内存

/*
 * @detail system v共享内存说明
 *    shmget //获取或创建共享内存
 *    shmat  //映射到进程内存空间
 *    shmdt  //解除共享内存映射
 *    shmctl //查看共享内存状态,与删除共享内存(IPC_RMID)
 * shmat将shm_nattch递增1, shmdt将shm_nattch递减1;
 * shctl(IPC_RMID)设置shm_perm.mode的SHM_DEST标志位; 如果shm_nattch==0,则删除共享内存; 如果shm_nattch>0,则再次调用shctl(IPC_RMID)会返回错误,errno=22,无效标识符
 * shmget同一进程多次调用同一共享内存不会影响shm_nattch;
 * 如果不调用shctl(IPC_RMID),则所有进程退出后,共享内存的shm_nattch==0,但是共享内存仍然存在; 可以通过ipcs查看
 * shmget(IPC_CREAT|) -> shmctl(IPC_RMID) ->shmget(IPC_CREAT|), 第二次调用shmget时, 虽然shmctl的引用计数器是0 && 共享内存未删除, 但仍认为共享内存不存在,走新建流程;
 */
//功能: 创建共享内存 system V共享内存暂不支持
//int mxx_shm_create(const char *pathname, int shmkey)
//{
//    return -1;
//}

////功能: 打开共享内存
//int mxx_shm_open(const char *pathname, int shmkey)
//{
//}
//
////功能: 共享内存映射到进程内存空间
//void * mxx_shm_mmap()
//{
//    return NULL;
//}
////功能: 共享内存从进程内存空间解除映射
//int mxx_shm_unmmap()
//{
//    return -1;
//}
//
//int mxx_shm_close(const char *pathname, int shmkey)
//{
//#ifdef WIN32 //暂时不支持 window系统
//    return MXX_IPC_SHM_KEY_ID_FAILED;
//#else //linux 
//    //获取共享内存key...
//    key_t tmpkey;
//    if(shmid<=0)
//        tmpkey=file_to_shmid(pathname, SHMID_SUBMASK); 
//    else
//        tmpkey=shmkey;
//    
//    //根据key获取共享内存在系统中的shmid,并删除共享内存
//    int shm_id=shmget(tmpkey, 0, SHM_FLAG);
//    if(shm_id!=-1)
//    {
//        /* @detail shmctl函数说明
//         //IPC_RMID 共享内存标记为删除,只有当最后一个attatch进程调用删除后(即shm_ds.shm_nattch==0),共享内存才会真正被删除
//         //调用进程必须是该共享内存的owner或privileged
//         //该函数会shm_perm.mode的SHM_DEST标志位, 可通过shmctl IPC_STAT查看
//         */
//        if(shmctl(shmid, IPC_RMID,0)<0)//!< 删除system V共享内存
//            return -1;//失败
//    }
//    
//    remove(pathname);//!< 删除对应的文件
//    return true;
//#endif
//}

//功能: 删除共享内存
//int mxx_shm_delete()
//{
//    return -1;
//}
//////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------
// 文件锁
//------------------------------------------------------------------------
/* linux 文件锁说明
 * 两种锁 posix(fcntl)与flock锁对比
 * 从内核实现的角度来看，每当创建一把文件锁的时候，逻辑如下:
 * 1).系统就会实例化一个struct file_lock对象，  
 * 2).file_lock对象会记录锁的相关信息：如锁的类型（共享锁，独占锁）、拥有这把锁的进程号、锁的标识（租赁锁，阻塞锁，POSIX锁，FLOCK锁），等等。
 * 3).最后把这个file_lock对象插入到被锁文件的inode.i_flock链表中，就完成了对该文件的加锁功能。
 * 4).要是其它进程想要对同一个文件加锁，那么它在将file_lock对象插入到inode.i_flock之前，会遍历该链表，
 *  如果没有发现冲突的锁，就将其插入到链表尾，表示加锁成功，否则失败。
 *
 * 至于为什么要将inode与file_lock以链表的形式关联起来，主要是考虑到用户有时可以对同一个文件加多个文件锁。
 * 例如：我们可以对同一个文件加多个共享锁；或者我们可以同时对文件加POSIX锁和FLOCK锁，这两种锁分别对应flock()和fcntl()两种系统调用函数；
 * 再或者可以通过多次调用fcntl()对同一个文件中的多个内容块加上POSIX记录锁。
 *
 * 下面讲下POSIX锁和FLOCK锁的一些区别：
 * 1. 
 *  POSIX锁和FLOCK锁分别是通过fcntl()和flock()系统调用完成的。
 *  虽然实现的原理上都差不多，都是生成一个file_lock对象并插入inode文件锁链表;
 *  但是POSIX锁是支持针对某一段文件内容进行加锁的，而FLOCK锁不支持。
 *
 * 2. POSIX锁可以重复加锁，即同一个进程，可以对同一个文件多次加同样一把锁。
 * 例如：第一次我对A文件的一个0~10的内容块加了一把独占锁，那么第二次同一个进程中我一样可以对这个A文件的0~10的内容块再加一把独占锁，这个有点像是递归加锁，但是我解锁时只需要解一次。
 * FLOCK锁则不同，如果你第一次对A文件加了一把独占锁，那么在同一个进程中你就不能对A文件再加一把锁了。这个区别其实只不过是在加锁的时候，遍历inode.i_flock链表时，发现存在PID相同的锁时，系统对于POSIX锁和FLOCK锁的具体处理手段不一样罢了。
 *
 * 3. 通过第2点，我们可以想象一下，POSIX锁和FLOCK锁在多线程环境下的不同。
 * 我们知道从Linux内核的视角来看，它是不区分所谓的进程和线程的，都不过是CPU调度队列中的一个个task_struct实例而已，所以不会对线程的场景进行专门的处理;
 * 也正以为如此，平时我们用的NPTL线程库也都是在用户态环境中模拟出来的，Linux内核并不直接支持。
 * 回到刚刚的话题，因为内核它在加锁的时候是看PID的，所以在内核看来多线程的加锁只不过是同一个进程（因为每个线程的PID都是一样的）在对同一个文件加多把锁。
 * 这样，多线程环境下的加锁行为就表现为：
 *       同一个进程中的多个线程可以对同一个文件加多次POSIX独占或共享锁;
 *       但是不可以对同一个文件加多次FLOCK独占锁（不过共享锁是可以加多次的）。
 *
 * 4. 在一个项目中使用了GPFS共享文件系统，我们在开发过程中发现，对于FLOCK锁只支持本地，而POSIX锁则可以支持跨主机加锁。
 * 例如：我们有两台独立的机器A和B，在A机器上有某个进程对文件f加POSIX独占锁，然后在B机器上当有某个进程想对f加POSIX独占锁时，就会失败。
 *    可是当我们使用FLOCK锁时，就发现两台机器对同一个文件加FLOCK锁是互不影响的，即A和B机器都可以对f加独占锁。
 * @see https://www.ibm.com/developerworks/cn/linux/l-cn-filelock/
 * @reference https://www.ibm.com/developerworks/cn/linux/l-cn-filelock/ 
 */
 /*
  * fcntl存在两种锁: 建议性锁和强制性锁
  * 其中强制性锁是非POSIX标准;
  * 建议性锁: 即其他进程可以无视锁的存在,继续读写文件;
  * 文件关闭后,系统会自动释放该进程在该文件上的锁;
  */
/**
 * @brief 文件加锁(建议锁)
 * @param
 *    [in]fileno:文件描述符,open函数返回;
 *    [in]locktype: 锁类型 MXX_WRLCK、MXX_RDLCK
 *    [in]blockflag: 阻塞标记, true-阻塞,直到获取锁; false-立即返回;
 *    [in]offset、len: 加锁范围的开始的位置与长度;offset=0,len=0表示整个文件加锁
 * @retval
 *    true-成功; false-失败;
 **/
bool mxx_lock_file(int fileno, unsigned int locktype, bool blockflag, unsigned int offset=0, unsigned int len=0)
{
    struct flock flck;
    flck.l_type = locktype;//!< F_RDLCK,F_WRLCK, F_UNLCK
    flck.l_whence = SEEK_SET;
    flck.l_start = offset;//!< 说明: 为加锁整个文件，通常的方法是将l_start 为0，l_whence 说明为SEEK_SET，l_len 为0。
    flck.l_len = len;
    flck.l_pid = getpid();
    if(blockflag)//阻塞标记
        return (fcntl(fileno,F_SETLKW,&flck)==0);
    else //立即返回
        return (fcntl(fileno,F_SETLK,&flck)==0);
}

//@brief 解锁
bool mxx_unlock_file(int fileno, unsigned int offset=0, unsigned int len=0)
{
    struct flock flck;
    flck.l_type = F_UNLCK;
    flck.l_whence = SEEK_SET;
    flck.l_start = offset;
    flck.l_len = len;
    flck.l_pid = getpid();
    return (fcntl(fileno,F_SETLK,&flck)==0);
}

//@brief 判断是否存在锁; true-存在锁; false-不存在锁;
bool mxx_exist_filelock(int fileno, unsigned int locktype, unsigned int offset, unsigned int len)
{
    struct flock flck;
    flck.l_type = locktype;
    flck.l_whence = SEEK_SET;
    flck.l_start = offset;
    flck.l_len = len;
    flck.l_pid = getpid();
    return (fcntl(fileno,F_GETLK,&flck)==0);
}

//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 根据路径产生system v(消息队列、信号量、共享内存) IPC通讯对象的id
 * @param
 *    [in]filename:文件名(不能是路径名), 进程对该文件必须有读写权限; 如果不存在,则创建;
 *    [in]id: 子序号,@see ftok子序号
 * @retval
 *    >0-system v ipc对象id; <0-失败;
 * @note
 *    该函数仅适用于产生system V的IPC对象id
 *    该函数不支持window;
 *    不同文件产生的key_t值可能相同;
 *    同一文件删除后重新创建会产生不同的key_t值;
 * @bug
 *    通过centos 6.8 64bit开发;
 *    未测试其他linux发布版支持情况;
 *    容易导致两个文件产生一个key
 **/
key_t file_to_shmkey(const char *filename, int key_submask)
{
#ifdef WIN32 //暂时不支持 window系统
    return MXX_IPC_SHM_KEY_ID_FAILED;
#else
    if(0!=access(filename, R_OK|W_OK))//没有读写权限 && 文件不存在
    {
        int fd=open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if(fd<0)//打开或创建文件失败
        {
            return MXX_IPC_SHM_KEY_ID_FAILED;
        }
        close(fd);
        fd=0;
    }
    
    /* @detail ftok说明
     //ftok产生key_t规则
     //1. id是子序号。虽然是int类型，但是只使用8bits(1-255）。
     //2. 在一般的UNIX实现中，是将文件的索引节点号取出，前面加上子序号得到key_t的返回值。
     //   如指定文件的索引节点号为65538，换算成16进制为0x010002，而你指定的ID值为38，换算成16进制为0x26，则最后的key_t返回值为0x26010002。
     //3.查询文件索引节点号的方法是： ls -i
     //4.当删除重建文件后，索引节点号由操作系统根据当时文件系统的使用情况分配，因此与原来不同，所以得到的索引节点号也不同。
     // 如果要确保key_t值不变，要么确保ftok的文件不被删除，要么不用ftok，指定一个固定的key_t值
     //@note 
     //   libc4和libc5函数原型: key_t ftok(char *pathname, char proj_id);
    */
    return ftok(filename, key_submask);
#endif
}

//@brief 产生共享内存key;  指定shmkey,则使用指定key; 否则使用pathname的inode产生key;
inline key_t to_shm_key(const char *pathname, int shmkey)
{
    key_t tmpkey;
    if(shmkey<=0)
        tmpkey=file_to_shmkey(pathname, SHMID_SUBMASK); 
    else
        tmpkey=shmkey;
    return tmpkey;
}

/////////////////////////////////////////////////////////////////////////////////////////////


/*
 * 共享内存说明:
 *
 */


//构造函数
IpcShmSysV::IpcShmSysV()
   :m_shmkey(-1),m_fd(MXX_INVALID_FD),m_shmid(MXX_INVALID_SHMID),m_shm_addr(NULL),m_shmsize(0),m_bCreated(true),b_init_succ_flag(false)
{
   memset(m_filepath, 0, sizeof(m_filepath));
   //m_fd=0;
   b_init_succ_flag=false;
}

//功能:析构函数
IpcShmSysV::~IpcShmSysV()
{
    //unmap
    if(NULL!=m_shm_addr)
    {
        shmdt(m_shm_addr);
    }
    m_shm_addr=NULL;
    //m_shmid=INVALID_SHMID; //shm_close需要使用,故不能置空
    m_shmsize=0;
    
    //删除(或关闭)共享内存
    if(m_shmid>=0)//!< 给最后一个移除共享内存的机会, 此处暂时不用判断m_bCreated
    {
        /*@detail shmctl函数说明
          //IPC_RMID 共享内存标记为删除,只有当最后一个attatch进程调用删除后(即shm_ds.shm_nattch==0),共享内存才会真正被删除
          //调用进程必须是该共享内存的owner或privileged
          //该函数会shm_perm.mode的SHM_DEST标志位, 可通过shmctl IPC_STAT查看
        */
        if(shmctl(m_shmid,IPC_RMID,0)<0)
        {
            printf("Error: IpcShmSysV::~IpcShmSysV shmctl(%d, IPC_RMID)失败! errno=[%d]\n", m_shmid, errno);
        }
    }
    m_shmid=MXX_INVALID_SHMID;
    
    //关闭文件
    if(m_fd>=0)
    {
        ::close(m_fd);
        m_fd=MXX_INVALID_FD;
    }
    
    //是否删除文件???
    //m_shmke m_filepath m_shmkey怎么处理??
    m_filepath[0]='\0';
}

int IpcShmSysV::clear_info()
{
    //unmap
    if(NULL!=m_shm_addr)
    {
        shmdt(m_shm_addr);
    }
    m_shm_addr=NULL;
    m_shmsize=0;
    
    //不能在此处删除(或关闭)共享内存
    if(m_shmid>=0)//只有创建进程才能删除共享内存, 防止多个进行部分青红皂白多次删除引起错误;
    {
        if(m_bCreated)
        { 
            if(shmctl(m_shmid,IPC_RMID,0)<0)//!< 不调用该函数,shm_nattch==0,共享内存依然存在,不会自动删除;
            {
                printf("Error: IpcShmSysV::clear_info shmctl(%d, IPC_RMID)失败! errno=[%d]\n", m_shmid,errno);
            }
        }
        //该函数调用后认为共享内存在本进程已经不存,m_bCreated==false, 也需要将m_shmid清空;
        m_shmid=MXX_INVALID_SHMID;//!< 
    }
    
    //关闭文件
    if(m_fd>=0)
    {
        ::close(m_fd);
        m_fd=MXX_INVALID_FD;
    }
    
    //m_shmkey与m_filepath不能清空;
    
    return 0;
}

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
int IpcShmSysV::shm_open(const char *filename, int shm_key, size_t shmsize)
{
    //int rc;
    
    if(is_init_succ()) return MXX_IPC_SHM_SUCC;
    
    //清理数据操作...
    clear_info();//!< 打开前,先清除对象数据,防止存在已打开或无效数据;
    
    snprintf(m_filepath,  sizeof(m_filepath)-1, "%s.shm", filename);//!< 保存文件名
    
  //打开文件...
  //O_CREAT: 如果不存在,则创建文件; O_RDWR:可读可写
  //mode: 权限user读写权限; group用户与others用户不可读写,即文件权限0400|0200
  int fd=open(m_filepath, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
  if(fd<0)//!< 打开文件失败
  {
      clear_info();
      
      b_init_succ_flag=false;
      memset(m_filepath, 0, sizeof(m_filepath));
      return MXX_OPENFILE_FAILED;
  }
  m_fd=fd;
  
  //获取shmkey...  指定key or 文件产生key
  key_t shmkey=-1;
  if(shm_key<=0)//如果没有传入key,则根据文件生成一个system V共享内存需要的key
      shmkey=file_to_shmkey(m_filepath, SHMID_SUBMASK);
  else
      shmkey=shm_key;
  if(shmkey<0)
  {
      clear_info();
      b_init_succ_flag=false;
      memset(m_filepath, 0, sizeof(m_filepath));
      return shmkey; 
  }
  m_shmkey=shmkey;
  
  
  bool b_created=true;
  //打开共享内存...  创建 or 打开
  //@note   shmget
  //shm大小限制见[SHMMIN,SHMMAX], @see /proc/sys/kernel/shmmax
  //SHM个数限制SHMNI, @see /proc/sys/kernel/shmmni
  //发现无论共享内存是否存在,shmflag=0777|IPC_CREAT,总是返回成功
  //情况1. centos 6.8测试发现: IPC_EXCL|IPC_CREAT打开的共享内存无法shmat(,0,SHM_FLAG)总是设置错误码13-permission denied;
  //       原因IPC_CREAT|IPC_EXCL的低24位数据为八进制000,即没有权限位,故必须知道PERM;
  //情况2. centoos 6.8测试 通过0777|IPC_EXCL|IPC_CREAT获取共享内存, 第一个进程是owner进行, 第二次调用的进程不是owner进行;
  //       owner进程删除共享内存;
  //       第一个进程shmctl(IPC_RMID)删除共享内存后; 第二个进程由于不是owner不进行删除,但是退出main后依然某个地方调用了shmctl(IPC_RMID)并返回无效id;
  //       原因不明??? 按正常逻辑第二个进程不是owner,不应该调用clear_info中的删除共享内存逻辑;
  //最终使用逻辑:
  //    使用0777|IPC_CREAT打开共享内存, 共享内存不在函数中显式删除(shmctl(IPC_RMID))
int sysV_shmid=shmget(shmkey, shmsize, 0777|IPC_EXCL|IPC_CREAT);
  if(sysV_shmid<0)//creat失败 || 没有权限
  {
      b_created=false;//!< 不是该对象创建的
      sysV_shmid=shmget(shmkey, 0, SHM_FLAG);
  }
  if(sysV_shmid<0)//创建失败 && 打开失败
  {
      clear_info();
      b_init_succ_flag=false;
      memset(m_filepath, 0, sizeof(m_filepath));
      return MXX_IPC_SHM_OPEN_FAILED;
  }
  m_shmid=sysV_shmid;
  m_shmsize=shmsize;
  m_bCreated=b_created;
  
  
  /* shmat说明
  //共享内存映射到本进程内存空间
  //@note: 
  //  linux中,即使shmid对应的共享内存被删除,该函数有可能内存映射成功; POSIX.1-2001不支持该种行为;
  //  产生的地址采用PAGE_SIZE对齐
  //  返回-1表示失败,并设置errno,
  //      EACCESS: 没有权限
  //      EIDRM: shmid已经被删除;
  //      EINVAL: shmid无效;
  //  成功后,该函数会更新shmid_ds结构(@see chmctl)
  //      shm_atime: 设置为当前时间
  //      shm_lpid:  调用进程的进程id
  //      shm_nattch: 递增1
  */
  void * shm_addr=shmat(sysV_shmid, NULL, 0);
  if((void *)-1L==shm_addr)
  {
     clear_info();
     b_init_succ_flag=false;
     memset(m_filepath, 0, sizeof(m_filepath));
     if(b_created)//当前对象创建还需要删除文件
     {
         //todo 删除文件...
     }
     
     return MXX_IPC_SHM_MMAP_FAILED;
  }
  m_shm_addr=shm_addr;

  
  struct shmid_ds info;
  shmctl(m_shmid,IPC_STAT,&info);
  
  //记录共享内存大小
  m_shmsize=info.shm_segsz;
  
  //
  if(b_created)
  {
      char szmsg[512];
      int n=sprintf(szmsg, "system V,shmkey=0x%08x,shmid=%10d shmsize=%ld", shmkey, sysV_shmid,info.shm_segsz);
      if(n>0)  write(m_fd, szmsg, n);
      
  }
  b_init_succ_flag=true;
  //m_shmid=shmid;
  //m_shmkey=shmkey;
  return MXX_IPC_SHM_SUCC;
}

/**
 * @brief 关闭共享内存
 * @param 无
 * @retval 
 *    0-成功; <0-失败;
 **/
int IpcShmSysV::shm_close()
{
    return clear_info();
}
/**
 * @brief 共享内存映射到进程空间
 * @param 无
 * @retval 
 *    NULL-失败; 其他-共享内存映射地址
 **/
unsigned char * IpcShmSysV::mmap()
{
  //if(!is_init_succ())  return NULL;
  
  //已经映射,则直接返回即可;
  if(NULL!=m_shm_addr) return (unsigned char *)m_shm_addr;
  
  //return this->shm_mmap();
  void * shm_addr=shmat(m_shmid, NULL, 0);
  if((void *)-1L==shm_addr)
  {
      return NULL;
  }
  
  struct shmid_ds info;
  shmctl(m_shmid,IPC_STAT,&info);
    
  m_shm_addr = (unsigned char *)shm_addr;
  m_shmsize=info.shm_segsz;
  
  return (unsigned char *)shm_addr;
}

 /**
  * @brief 解除共享内存映射
  * @param 无
  * @retval 
  *    0-成功; <0-失败;
  **/
int IpcShmSysV::unmap()
{
  //if(!is_init_succ()) return  0;
  
  if(NULL==m_shm_addr) return 0;
  //return this->unmap();
  
  if(NULL!=m_shm_addr)
  {
      shmdt(m_shm_addr);
      m_shm_addr=NULL;
      m_shmsize = 0;
  }
  
  return 0;
}

// @brief 获取共享内存关联计数
int IpcShmSysV::get_shm_nattch()
{
    if(m_shmid<0) return -1;
    
    struct shmid_ds info;
    shmctl(m_shmid,IPC_STAT,&info);
    return info.shm_nattch;    
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//unsigned char * IpcShmSysV::shm_mmap()
//{
//    (void *) shm_addr=shmat(sysV_shmid, NULL, 0);
//    if((void *)-1L==shm_addr)
//    {
//        return NULL;
//    }
//    
//    
//    struct shmid_ds info;
//    shmctl(m_shmid,IPC_STAT,&info);
//    
//    m_shm_addr = shm_addr;
//    m_shmsize=info.shm_segsz;
//  return m_shm_addr;
//}
//
////
//int IpcShmSysV::shm_unmap()
//{
//  if(NULL!=m_shm_addr)
//  {
//        shmdt(m_shm_addr);
//  }
//  m_shm_addr=NULL;
//  m_shmid=INVALID_SHMID;
//  //m_shmsize=0;
//  return 0;
//}
//
//int IpcShmSysV::shm_close()
//{
//    //int shmid = shmget(shmkey,0,BUSHM_FLAG);
//    if (m_shmid>=0)
//    {
//        if (shmctl(shmid,IPC_RMID,0)==-1)
//            return(false);
//    }
//    m_shmid=INVALID_SHMID;
//    return 0;
//}

/**
 * @brief 关闭文件
 * @param
 *    [in]remove_flag: true-删除磁盘shm文件; false-仅关闭文件,不删除磁盘文件
 * @retval true-成功; false-失败;
 **/
bool IpcShmSysV::file_close(bool remove_flag/*=false*/)
{
    if(m_fd>0)
    {
        ::close(m_fd);
    }
    m_fd=MXX_INVALID_FD;
    if(remove_flag &&('\0'!=m_filepath[0]))
    {
        remove(m_filepath);
    }
    return true;    
}

