#include "ipc_shm.h"

#include "gtest/gtest.h"

#include <stdio.h>

#include <semaphore.h> //POSIX信号量头文件
#include <errno.h>
#include <string.h>

#define OUT_INFO_MSG(format,...) printf(format"\n", ##__VA_ARGS__)


void clear_errno() { errno=0; }

//测试POSIX信号量: 共享内存 open操作
TEST(posix_ipc_shm, ipc_shm_open)
{
     printf("---------posix: ipc_shm:  ipc_shm_open---------\n");
     int rc=0;
     
     int shm_fd,shm_fd2;
     char filepath[256]="test.shm_px";
     int shm_size=0;

     rc=mxx_shm_open(filepath, shm_size);
     shm_fd=rc;
     OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc,errno, strerror(errno));
     EXPECT_GT(rc,0);//创建成功;
      
     clear_errno();
     rc=mxx_shm_open(filepath,shm_size);
     shm_fd2=rc;
     OUT_INFO_MSG("second: mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno));
     EXPECT_GT(rc,0);//共享内存打开成功
     EXPECT_GT(shm_fd2, shm_fd);//第二次打开的文件描述符比第一次打开的文件描述符大; 即不是同一个文件描述符;

    clear_errno();
    rc=mxx_shm_close(filepath);
    OUT_INFO_MSG("mxx_shm_close(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno));
    EXPECT_EQ(rc,0);//关闭文件描述符;

    clear_errno();
    strcpy(filepath, "/test1.shm_px");
    rc=mxx_shm_open(filepath,shm_size);
    shm_fd=rc;
    OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno));
    EXPECT_GT(rc, 0);//打开成功

    clear_errno();
    strcpy(filepath, "./test2.shm_px");
    rc=mxx_shm_open(filepath,shm_size);
    shm_fd=rc;
    OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno));
    EXPECT_GT(rc, 0);//打开成功

    clear_errno();
    strcpy(filepath, "./test3.shm_px");
    int tmp_shm_size=256;
    rc=mxx_shm_open(filepath, tmp_shm_size);
    shm_fd=rc;
    OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, tmp_shm_size, rc, errno, strerror(errno));
    EXPECT_GT(rc, 0);//打开成功
   
    clear_errno(); 
    strcpy(filepath, "/tmp/test3.shm_px");
    rc=mxx_shm_open(filepath,shm_size);
    shm_fd=rc;
    OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno)); 
    EXPECT_LT(rc, 0);//打开失败

    strcpy(filepath, "/home/git_user/git_server_repo/program_trading_repo/trunk/2_source/common_utils/demo_dir/test4.shm_px");
    rc=mxx_shm_open(filepath,shm_size);
    shm_fd=rc;
    OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno));
    EXPECT_LT(rc, 0);//打开失败

    clear_errno();
}

typedef struct _shm_struct
{
   int pid;
   int count;
} ShmCtrl;

TEST(posix_ipc_shm, ipc_shm_mmap)
{
  int rc;
  int shm_fd;
  ShmCtrl *shm_ptr=NULL;

  unsigned char *ptr;

  int shm_size=sizeof(ShmCtrl)*2;

  char filepath[256]="test_mmap.shm_px";
  rc=mxx_shm_open(filepath,shm_size);
  shm_fd=rc;
  OUT_INFO_MSG("mxx_shm_open(%s,%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, shm_size, rc, errno, strerror(errno)); 
  if(rc<0)//不用继续了,打开共享内存失败
  {
  }

  int offset=0;
  int size=sizeof(ShmCtrl);
  ptr=mxx_shm_alloc(shm_fd, 0, sizeof(ShmCtrl));
  OUT_INFO_MSG("mxx_shm_alloc(%s, offset=%d, size=%d), rc=[%d], errno=[%d], strerr=[%s]", filepath, offset, size, rc, errno, strerror(errno));
  EXPECT_NE(ptr, (unsigned char *)NULL);

  OUT_INFO_MSG("set shm value...");
  shm_ptr=(ShmCtrl *)ptr;
  shm_ptr->pid=getpid();
  shm_ptr->count=100;
  OUT_INFO_MSG("pid=[%d], count=[%d]", shm_ptr->pid, shm_ptr->count);
  
  
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.
    return RUN_ALL_TESTS();
}
