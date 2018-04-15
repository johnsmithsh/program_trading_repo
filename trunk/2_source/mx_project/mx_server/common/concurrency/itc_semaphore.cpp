#include "itc_semaphore.h"
#include <math.h>
#include <time.h>

//功能:初始化信号量; shared:0-(进程内)线程共享; 1-(父子)进程间信号量; init_value:信号量初始值;
int ItcSem::init(int shared, int init_value)
{
    int rc;
    rc=sem_init(&m_sem, (0==shared)?0:1, init_value);
    b_init_succ_flag=(0==rc);
    return (0==rc) ? MXX_SEM_SUCC : MXX_SEM_FAILED;
}


//功能:构造函数
ItcSem::ItcSem()
{
   init(0, 1);
}

//功能:构造函数; shared:0-(进程内)线程共享; 1-(父子)进程间信号量; init_value:信号量初始值;
ItcSem::ItcSem(int shared, int init_value)
{
   init(shared, init_value);
}


ItcSem::~ItcSem()
{
   if(is_init_succ())
   {
      int rc;
      rc=sem_destroy(&m_sem);
      b_init_succ_flag=false;
   }
}

//功能:递减信号量,即等待事件; 返回值:-1-没有等到信号量; 0-等到需要的信号量;
int ItcSem::wait()
{
   if(!is_init_succ()) return MXX_SEM_ERROR_INIT_FAILED;
   int rc;
   
   //errno: EINTR 被signal 中断;
   //       EINVAL sem不是有效的semaphore
   rc=sem_wait(&m_sem);  
   if(0==rc)//获取到信号量
      return MXX_SEM_SUCC;
   else //没有获取信号量
      return MXX_SEM_FAILED;
}

//功能:递减信号量(即等待事件); milli_second:超时事件,单位:毫秒; 返回值: 0-等到需要的信号量; <0-没有等到需要的信号量;
int ItcSem::try_wait(double milli_second/*=0.0*/)
{
   //初始化失败
   if(!is_init_succ()) return MXX_SEM_ERROR_INIT_FAILED;

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

       //errno: EINTR 被signal 中断;
       //       EINVAL sem不是有效的semaphore
       //       EAGAIN 同try_wait
       //       ETIMEDOUT  超时
       rc=sem_timedwait(&m_sem, &ts);
       if(0==rc) //获取到信号量
          return MXX_SEM_SUCC;
       else
          return MXX_SEM_FAILED;
    }
    else
    {
      //errno: EINTR   被signal 中断;
      //       EINVAL  sem不是有效的semaphore
      //       EAGAIN  信号递减不能立刻执行,又不能阻塞;
      rc=sem_trywait(&m_sem);
      if(0==rc)//获取到信号量
        return MXX_SEM_SUCC;
      else 
        return MXX_SEM_FAILED;
    }
}

//功能: 信号量递增, 即发出事件通知; 0-成功; <0失败;
int ItcSem::post()
{
    //初始化失败
    if(!is_init_succ()) return MXX_SEM_ERROR_INIT_FAILED;

    //errno: EINVAL  无效semaphore
    //       EOVERFLOW 信号量溢出(超过最大值)
    int rc=sem_post(&m_sem);
    if(0==rc)//
      return MXX_SEM_SUCC;
    else
      return MXX_SEM_FAILED;
}

