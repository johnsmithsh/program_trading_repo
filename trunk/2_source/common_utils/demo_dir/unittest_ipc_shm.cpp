#include "ipc_shm.h"

#include "gtest/gtest.h"

#include <stdio.h>

#include <semaphore.h> //POSIX信号量头文件
#include <errno.h>
#include <string.h>

#define OUT_INFO_MSG(format,...) printf(format"\n", ##__VA_ARGS__)

typedef struct _shm_struct
{
   int pid;
   int count;
} ShmCtrl;

void clear_errno() { errno=0; }

//测试POSIX信号量: 共享内存
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


int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.
    return RUN_ALL_TESTS();
}
