#include "os_path.h"

#include "gtest/gtest.h"

#include <stdio.h>

#include <semaphore.h> //POSIX信号量头文件
#include <errno.h>
#include <string.h>

//验证posix信号量 semaphore 的使用情况

//测试POSIX信号量: 内存信号量
TEST(posix_sem, mem_sem)
{
     printf("---------posix: mem_sem:  normalling use---------\n");
     int rc=0;
     sem_t sem;

     int sem_value;
     rc=sem_init(&sem, 0, 1);
     EXPECT_EQ(rc,0);
     printf("sem_init, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));

     //获取value
     sem_value=0;
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 1);

     rc=sem_post(&sem);
     EXPECT_EQ(rc,0);
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 2);

     rc=sem_post(&sem);
     EXPECT_EQ(rc,0);
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 3);

     printf("\nnow sem_destroy...\n");
     rc=sem_destroy(&sem);
     EXPECT_EQ(rc,0);
     rc=sem_getvalue(&sem, &sem_value);//sem_destroy后sem依然可以打开和使用;
     printf("after sem_destroy=>sem_getvalue, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     rc=sem_wait(&sem);
     EXPECT_EQ(rc,0);
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 2);
     printf("after sem_destroy=>sem_wait, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));

     printf("\nnow sem_close...\n");
     rc=sem_close(&sem);
     EXPECT_NE(rc,0);
     if(rc<0)
        printf("after sem_destroy=>sem_close, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);//sem_destroy后sem依然可以打开和使用;
     printf("after sem_destroy=>sem_close=>sem_getvalue, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     rc=sem_wait(&sem);
     EXPECT_EQ(rc,0);
     printf("after sem_destroy=>sem_close=>sem_wait, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 1);
     printf("after sem_destroy=>sem_close=>sem_wait, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));

     printf("\nAnswer:  \n");
     printf("   sem_t is still can use after sem_destroy()\n");
     printf("   after sem_destroy(), then sem_close() is failed with rc==-1,errno==22\n");
     printf("          but sem_wait(),sem_getvalue() will return rc==0,and errno==22\n");
     printf("============================end======================================\n");

}

//测试案例: 测试信号量初始信号为0; sem_destroy()=>sem_close();
TEST(posix_sem, mem_sem_1)   
{
     printf("\n-------------if sem is seted to 0 initially, then it can still be usefully after sem_destroy() ----------------\n");
     sem_t sem;
     int sem_value;
     int rc;
     rc=sem_init(&sem, 0, 0);
     EXPECT_EQ(rc,0);
     printf("sem_init, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 0);
     
     sem_post(&sem);
     printf("sem_post, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
 
     printf("\nnow sem_destroy...\n");
     rc=sem_destroy(&sem);
     EXPECT_EQ(rc,0);
     printf("sem_destroy, rc=[%d], errno=[%d], strerr=[%s]\n", rc, errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);//sem_destroy后sem依然可以打开和使用;
     printf("after sem_destroy=>sem_getvalue, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     rc=sem_post(&sem);
     EXPECT_EQ(rc,0);
     printf("after sem_destroy=>sem_post, rc=[%d], errno=[%d], strerr=[%s]\n", rc,errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);
     EXPECT_EQ(rc,0);
     EXPECT_EQ(sem_value, 2);
     printf("after sem_destroy=>sem_post=>sem_gevalue, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));

     rc=sem_close(&sem);
     printf("sem_close, rc=[%d], errno=[%d], strerr=[%s]\n", rc, errno, strerror(errno));

     rc=sem_wait(&sem);
     EXPECT_EQ(rc,0);
     printf("...sem_wait, rc=[%d], errno=[%d], strerr=[%s]\n", rc,errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);
     printf("..sem_wait=>sem_gevalue, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     EXPECT_EQ(rc, 0);
     EXPECT_EQ(sem_value, 1);

     rc=sem_wait(&sem);
     EXPECT_EQ(rc,0);
     printf("...sem_wait, rc=[%d], errno=[%d], strerr=[%s]\n", rc,errno, strerror(errno));
     rc=sem_getvalue(&sem, &sem_value);
     printf("..sem_wait=>sem_gevalue, rc=[%d],sem_value=[%d], errno=[%d], strerr=[%s]\n", rc, sem_value, errno, strerror(errno));
     EXPECT_EQ(rc, 0);
     EXPECT_EQ(sem_value, 0);

     printf("now another sem_wait, it should be block...\n");
     rc=sem_wait(&sem);
     EXPECT_EQ(rc,0);
     printf("...sem_wait, rc=[%d], errno=[%d], strerr=[%s]\n", rc,errno, strerror(errno));
}


int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.
    return RUN_ALL_TESTS();
}
