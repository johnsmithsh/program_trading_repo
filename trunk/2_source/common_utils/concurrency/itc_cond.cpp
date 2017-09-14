#include "itc_cond.h"

#include <time.h>
#include <math.h>


//pthread_cond_init,pthread_cond_signal,pthread_cond_broadcast,pthread_cond_wait只返回0，不会返回其他错误码。
//也就是说这几个函数每次调用都会成功，编程时不用检查返回值。
//但pthread_cond_timedwait和pthread_cond_destroy会返回错误码，需要注意！
//注:上面的情况没有证实过;

//功能: 初始化条件变量及关联的互斥量
//参数: shared-共享标记; 0-线程共享; 1-线程;
//注: 尚未明白条件变量怎么进程共享; 故现在仅支持线程共享;
int ItcCond::init(int shared)
{
   int rc;
   ////对一个已经初始化的attr调用pthread_condarrt_init,会导致不可预料的结果;
   ////errno:
   ////    ENOMEM  无效内存
   //rc=pthread_condattr_init(&m_attr);
   //
   ////设置线程共享属性;
   ////errno:
   ////    EINVAL attr无效
   ////    EINVAL 参数值无效
   //if(0==shared)//(进程内)线程共享
   //  rc=pthread_condattr_setpshared(&m_attr, PHTREAD_PROCESS_PRIVATE);
   //else //进程共享
   //  rc=pthread_condattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);
   //
   ////不设置了,还是采用系统默认始终吧!  
   //rc=pthread_condattr_setclose(&m_attr, );

   //设置保护条件变量的互斥量
   pthread_mutexattr_t attr;
   rc=pthread_mutexattr_init(&attr);
   if(0==shared)//(进程内)线程共享
      rc=pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
   else //(进程间)线程共享
      rc=pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
   if(rc<0)//初始化互斥量属性失败
   {
      return MXX_COND_INIT_MUTEX_FAILED;
   }

   rc=pthread_mutex_init(&m_mutex, &attr);
   if(rc<0)//初始化互斥量失败
     return MXX_COND_INIT_MUTEX_FAILED;
   rc=pthread_mutexattr_destroy(&attr);//销毁互斥量属性
   
   //初始化条件变量
   //errno/return???:
   //    EAGAIN 缺少初始条件变量所需的资源;
   //    ENOMEM 存在无效内存;
   rc=pthread_cond_init(&m_cond, NULL);//linux thread没有实现,故cond_attr置空
   b_init_succ_flag = (0==rc);
   if(0==rc)
     return MXX_COND_SUCC;
  else//初始化条件变量失败
  {
     //pthread_condattr_destroy(&m_attr);
     rc=pthread_mutex_destroy(&m_mutex);//销毁互斥量
     return MXX_COND_FAILED;
  }
}

//功能: 构造函数
ItcCond::ItcCond()
{
   init(0); 
}

//功能:构造函数; shared-共享标记;
ItcCond::ItcCond(int shared)
{
  init(0);
}

//功能: 析构函数
ItcCond::~ItcCond()
{
   if(is_init_succ())
   {
      int rc;

      //销毁互斥量
      rc=pthread_mutex_destroy(&m_mutex);
    
      //销毁条件变量
      //  销毁条件变量是安全的, if条件变量没有阻塞线程; 如果条件变量阻塞线程,则销毁条件变量的结果不可预料;
      //return:
      //    EBUSY  试图销毁一个正在使用的条件变量(如有线程被阻塞),是否支持依赖具体实现;
      //    EINVAL 条件变量无效;
      rc=pthread_cond_destroy(&m_cond);

      //rc=pthread_condattr_destroy(&m_attr);
      
      //return:
      //    EBUSY  试图销毁一个正在使用的互斥量(如有线程被阻塞),是否支持依赖具体实现;
      //    EINVAL 条件变量无效;
      rc=pthread_mutex_destroy(&m_mutex);
      b_init_succ_flag=false;
   }
}

//功能: 加锁; 一般在调用条件变量wait前,需要先加锁; 否则出现不可预料的行为;
//返回值: 0-成功; <0-失败;
int ItcCond::lock()
{
   if(!is_init_succ())
     return -1;
   int rc=pthread_mutex_lock(&m_mutex);//pthread_mutex_lock返回0成功;其他失败
   return (0==rc) ?  MXX_COND_SUCC : MXX_COND_MUTEX_LOCK_FAILED;
}

//功能: 解锁;一般在条件变量cond_wait退出后解锁
//返回值: 0-成功; <0-失败;
int ItcCond::unlock()
{
   if(!is_init_succ())
     return -1;
   int rc=pthread_mutex_unlock(&m_mutex);//不判断返回值了
   return (0==rc) ? MXX_COND_SUCC : MXX_COND_MUTEX_UNLOCK_FAILED;
}


/*
 * pthread_cond_wait调用相当复杂，它是如下执行序列的一个组合: 
 * 1. 释放互斥锁 并且 线程挂起（这两个操作是一个原子操作）
 * 2. 线程获得信号，尝试获得互斥锁后被唤醒;
 * 调用pthread_cond_signal给条件变量发送信号时，如果当时没有线程在等待这个条件变量，信号将被丢弃。
 * 如果"释放互斥锁"和"线程挂起"不是一个原子操作，那么pthread_cond_wait线程在"释放互斥锁"和"线程挂起"之间，如果有信号一旦发生，程序就会错失一次条件变量变化
 * */
//功能:等待事件发生
//返回值: 0-事件发生; <0-失败;
//注:  调用该函数前必须确保调用该ItcCond::lock()成功加锁;
//     调用该函数后必须确保ItccCond::unlock()释放锁;
int ItcCond::wait()
{
   int rc;
   int ret_code=0;
   if(!is_init_succ())
     return MXX_COND_INIT_FAILED;

   //rc=pthread_mutex_lock(&m_mutex);
   //if(rc<0)
   //{
   //    return -1;
   //}

   //此处应该有一个检查条件操作,如果不满足,则wait

   //pthread_wait调用时,必须有mutex保护,否则会造成无法预料的行为;
   //pthread_cond_wait内部会解锁，然后等待条件变量被其它线程激活
   //返回值:
   //   ETIMEOUT  超时
   //   EINVAL cond、mutex、timespec之一无效;
   //   EPERM 调用wait时,当前线程没有拥有mutex;
   rc=pthread_cond_wait(&m_cond, &m_mutex);   
   if(0==rc)
   {
      //此处再次检查条件,满足则返回MXX_COND_SUCC,否则返回MXX_COND_FAILED

      ret_code=MXX_COND_SUCC;
   }
   else
      ret_code=MXX_COND_FAILED;

   //rc=pthread_mutex_unlock(&m_mutex);
   return ret_code;
}

//功能:等待事件发生,可以指定超时时间
//返回值: 0-事件发生; <0-失败;
//注:  调用该函数前必须确保调用该ItcCond::lock()成功加锁;
//     调用该函数后必须确保ItccCond::unlock()释放锁;
int ItcCond::try_wait(double milli_second)
{
   int rc;
   int ret_code;
   if(!is_init_succ())
     return MXX_COND_INIT_FAILED;

   //rc=pthread_mutex_lock(&m_mutex);
   //if(rc<0)
   //{
   //   return -1;
   //}

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

       //此处应该有一个检查条件操作,如果不满足,则wait

       //pthread_cond_wait内部会解锁，然后等待条件变量被其它线程激活
       rc=pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
   }
   else
   {
      rc=pthread_cond_wait(&m_cond, &m_mutex);
   }

   if(0==rc)//获取信号
   {
      //此处再次检查条件,满足则返回MXX_COND_SUCC,否则返回MXX_COND_FAILED
      ret_code=0;
   }
   else//发生错误;
   {
      ret_code=MXX_COND_FAILED;
   }

   //rc=pthread_mutex_unlock(&m_mutex);

   return ret_code;
}

//功能:唤醒一个被挂起的线程
//返回值: 0-成功; <0-失败;
//注:  调用该函数前必须确保调用该ItcCond::lock()成功加锁;
//     调用该函数后必须确保ItccCond::unlock()释放锁;
int ItcCond::signal()
{
   int rc;
   if(!is_init_succ())
     return MXX_COND_INIT_FAILED;

   //rc=pthread_mutex_lock(&m_mutex);
   //if(rc<0)
   //  return -1;

   rc=pthread_cond_signal(&m_cond);//唤醒一个休眠线程

   //rc=pthread_mutex_unlock(&m_mutex);

   return (0==rc) ? MXX_COND_SUCC : MXX_COND_FAILED;
   
}

//功能:唤醒所有被挂起的线程
//返回值: 0-成功;<0-失败;
//注:  调用该函数前必须确保调用该ItcCond::lock()成功加锁;
//     调用该函数后必须确保ItccCond::unlock()释放锁;
int ItcCond::broadcast()
{
   int rc;
   if(!is_init_succ())
     return MXX_COND_INIT_FAILED;

   //rc=pthread_mutex_lock(&m_mutex);
   //if(rc<0)
   //  return -1;

   rc=pthread_cond_broadcast(&m_cond);//唤醒所有休眠线程

   //rc=pthread_mutex_unlock(&m_mutex);

   return (0==rc) ? MXX_COND_SUCC : MXX_COND_FAILED;
}

