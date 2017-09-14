#include "os_path.h"

#include "gtest/gtest.h"

#include <stdio.h>

#include <pthread.h> //POSIX信号量头文件
#include <errno.h>
#include <string.h>

//获取错误码
#define GetExecResult(errcode)  errcode=errno
//输出执行结果
#define OutExecInfo(rc, errcode,fmt,...)  printf("  [exec_result] "fmt";rc=[%d], errcode=[%d],strerr=[%s]\n", ##__VA_ARGS__, rc, errcode,strerror(errcode))

typedef struct __thread_arg
{
   int id;
} ST_THREAD_ARG;

int g_value;//公共变量
pthread_mutex_t g_mutex;
pthread_cond_t g_cond;
pthread_t g_thread;
void* thread_cond_wait(void *thread_arg)
{

   int last_value=g_value;
   int rc;
   ST_THREAD_ARG *pArg=(ST_THREAD_ARG *)thread_arg;
   for(;;)
   {
        pthread_mutex_lock(&g_mutex);
        if(g_value==999)
        {
            pthread_mutex_unlock(&g_mutex);
            break;
        }
        rc=0;
        if(last_value==g_value)
        {
           rc=pthread_cond_wait(&g_cond, &g_mutex);
           if(rc<0)
           {
              printf("Error: thread[%d] pthread_cond_wait error! rc=[%d], errno=[%d], strerr=[%s]\n", pArg->id, rc, errno, strerror(errno));
           }
        }
        last_value=g_value;

        pthread_mutex_unlock(&g_mutex);
        printf("thread[%d]  value=[%d]\n", pArg->id, last_value);

   }
   
   
}


//说明:
//  验证pthread条件变量在不同情况下的行为和返回值、错误码

//测试POSIX信号量: 内存信号量
TEST(posix_cond, cond_init)
{
     printf("---------pthread_cond:init---------\n");

     pthread_cond_t m_cond;
     int rc;
     int errcode;

     printf("测试: 在一个条件变量上重复初始化...\n");
     rc=pthread_cond_init(&m_cond, NULL);
     OutExecInfo(rc, errno, "第一次初始化:");
     EXPECT_EQ(rc, 0);
     EXPECT_EQ(errno, 0);
     rc=pthread_cond_init(&m_cond, NULL);
     OutExecInfo(rc, errno, "第二次初始化:");
     EXPECT_EQ(rc, 0);//第二次应该初始化失败
     EXPECT_EQ(errno, 0);
     printf("结果: 虽然连续多次初始化条件变量没有发现什么异常,还是不要这么做了!\n\n");

     //产生core dump
     printf("测试: 在无效条件变量上初始化...\n");
     //rc=pthread_cond_init(NULL,NULL);
     //OutExecInfo(rc, errno, "第一次初始化:");
     //EXPECT_EQ(rc, 0);
     //EXPECT_EQ(errno, 0);
     printf("结果: 产生core dump!\n\n");

     sleep(1);
     printf("测试: 销毁一个正在阻塞线程的条件变量...\n");
     rc=pthread_cond_destroy(&g_cond);
     errcode=errno;
     OutExecInfo(rc, errno, "销毁调用完成:"); 
     EXPECT_EQ(rc, EBUSY);
     EXPECT_EQ(errcode,0);
     printf("结果: 销毁阻塞线程的条件变量,rc=[EBUSY], errno=[0]\n");

    

}

//测试案例: 测试信号量初始信号为0; sem_destroy()=>sem_close();
TEST(posix_sem, mem_sem_1)   
{
}


int main(int argc, char **argv)
{
   int rc;
   printf("测试前准备: 初始化测试使用的互斥量...\n");
   rc=pthread_mutex_init(&g_mutex, NULL);
   OutExecInfo(rc, errno, "初始化互斥量完成!");
   if(rc<0)
   {
       printf("Error: 初始化互斥量失败!");
       return 0;
   }
   printf("测试前准备: 初始化条件变量...\n");
   rc=pthread_cond_init(&g_cond, NULL);
   OutExecInfo(rc, errno, "初始化条件变量完成!");
   if(rc<0)
   {
       printf("Error: 初始化条件变量失败!");
       return 0;
   }

   ST_THREAD_ARG thread_arg;
   thread_arg.id=0;

   rc=pthread_create(&g_thread,NULL, thread_cond_wait, &thread_arg);
   OutExecInfo(rc, errno, "测试前准备:创建线程完成!");
   if(rc<0)
   {
      printf("Error: 创建线程失败!");
      return 0;
   }
   testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.
   RUN_ALL_TESTS();

   rc=pthread_cancel(g_thread);
   OutExecInfo(rc, errno, "测试完成:停止线程!");
   sleep(3);

   rc=pthread_mutex_destroy(&g_mutex);
   OutExecInfo(rc, errno, "测试完成:销毁互斥量!");

   rc=pthread_cond_destroy(&g_cond);
   OutExecInfo(rc, errno, "测试完成:销毁条件变量!");
}
