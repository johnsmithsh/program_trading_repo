#include "itc_cond.h"

#include <time.h>
#include <math.h>

int ItcCond::init(int shared)
{
   int rc;
   //对一个已经初始化的attr调用pthread_condarrt_init,会导致不可预料的结果;
   //errno:
   //    ENOMEM  无效内存
   rc=pthread_condattr_init(&m_attr);

   //设置线程共享属性;
   //errno:
   //    EINVAL attr无效
   //    EINVAL 参数值无效
   if(0==shared)//(进程内)线程共享
     rc=pthread_condattr_setpshared(&m_attr, PHTREAD_PROCESS_PRIVATE);
   else //进程共享
     rc=pthread_condattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);
   
   //不设置了,还是采用系统默认始终吧!  
   //rc=pthread_condattr_setclose(&m_attr, );

   //设置保护条件变量的互斥量
   pthread_mutexattr_t attr;
   rc=pthread_mutexattr_init(&attr);
   if(0==shared)//(进程内)线程共享
      rc=pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
   else //(进程间)线程共享
      rc=pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
   rc=pthread_mutex_init(&m_mutex, &attr);
   rc=pthread_mutex_destroy(&attr);//销毁互斥量属性
   
   //初始化条件变量
   //errno:
   //    EAGAIN 缺少初始条件变量所需的资源;
   //    ENOMEM 存在无效内存;
   rc=pthread_cond_init(&m_cond, &m_attr);
   b_init_succ_flag = (0==rc);
   if(0==rc)
     return MXX_COND_SUCC;
  else
  {
     pthread_condattr_destroy(&m_attr);
     return MXX_COND_FAILED;
  }
}

ItcCond::ItcCond()
{
   init(0); 
}

ItcCond::ItcCond(int shared)
{
  init(shared);
}

ItcCond::~ItcCond()
{
   if(is_init_succ())
   {
      int rc;

      //销毁互斥量
      rc=pthread_mutex_destroy(m_mutex);
    
      //销毁条件变量
      //  销毁条件变量是安全的, if条件变量没有阻塞线程; 如果条件变量阻塞线程,则销毁条件变量的结果不可预料;
      //errno:
      //    EBUSY  试图销毁一个正在使用的条件变量;
      //    EINVAL 条件变量无效;
      rc=pthread_cond_destroy(&m_cond);

      rc=pthread_condattr_destroy(&m_attr);
      b_init_succ_flag=false;
   }
}


int ItcCond::wait()
{
   int rc;

   rc=pthread_mutex_lock(&m_mutex);
   if(rc<0)
   {
      return -1;
   }

   //pthread_wait调用时,必须有mutex保护,否则会造成无法预料的行为;
   //
   rc=pthread_cond_wait(&m_cond, &m_mutex);   

   rc=pthread_mutex_unlock(&m_mutex);
}


int ItcCond::try_wait(double milli_second)
{
   int rc;
   if(milli_second>0.000001)
    {
       timespec ts;
       double fractpart=0.0,intpart=0.0;
       fractpart=modf(milli_second/1000, &intpart);//取出整数部分和小数部分
       long sec=(int)intpart;
       long nsec=(int)fractpart*100000000;//纳秒

       ts.tv_sec=ts.tv_nsec=0;
       clock_gettime(CLOCK_REALTIME, &ts);
       ts.tv_sec+=sec;//秒
       ts.tv_nsec+=nsec;//纳秒

       rc=pthread_cond_timedwait(&m_cond, &m_mutex);
   }
   else
   {
      rc=pthread_cond_trywait(&m_cond, &m_mutex);
   }
}


int ItcCond::signal()
{
}

int ItcCond::broadcase()
{
}

