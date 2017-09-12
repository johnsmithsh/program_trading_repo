#include "itc_mutex.h"

#ifdef __STD_BOOST_MACRO__ //使用BOOST封装锁

#include "boost/thread/mutex.hpp"

void Thread_Mutex::lock()
{
    m_mutex.lock();
}

void Thread_Mutex::unlock()
{
    m_mutex.unlock();
}


#endif //end of __STD_BOOST_MACRO__


#ifdef __STD_POSIX_MACRO__ //使用POSXI mutex

#include <time.h>
#include <math.h>

ItcMutex::ItcMutex()
{
   //
   int rc;
   rc=pthread_mutex_init(&m_mutex,NULL);//初始化设置
   if(0!=rc)//初始化失败???
   {
   }
   m_init_succ_flag=(0==rc);//设置初始化成功标记
}

ItcMutex::~ItcMutex()
{
    if(is_init_succ())
    {
       pthread_mutex_destroy(&m_mutex);
    }
    m_init_succ_flag=false;
}

int ItcMutex::lock()
{
    int rc;
    rc=pthread_mutex_lock(&m_mutex);//pthread_mutex_lock返回0成功;其他失败
    return (0==rc) ? 0 : -1;
}

int ItcMutex::try_lock(double milli_second/*=0.0*/)
{
    //初始化失败
    if(!is_init_succ()) return -2;

    int rc;
    
    if(milli_second>0.000001)
    {
       timespec ts;
       double fractpart=0.0,intpart=0.0;
       fractpart=modf(milli_second, &intpart);//取出整数部分和小数部分

       ts.tv_sec=ts.tv_nsec=0;
       clock_gettime(CLOCK_REALTIME, &ts);
       ts.tv_sec+=(int)(intpart/1000);//秒
       ts.tv_nsec+=((int)intpart%1000)*1000000 + (int)(fractpart*1000000);//纳秒

       rc=pthread_mutex_timedlock(&m_mutex, &ts);
       if(0==rc) //加锁成功
          return 0;
       else
          return -1;
    }
    else
    {
       rc=pthread_mutex_trylock(&m_mutex);
       return (0==rc) ? 0 : -1;
    }
}

int ItcMutex::unlock()
{
   if(!is_init_succ()) return -1;
   int rc;
   rc=pthread_mutex_unlock(&m_mutex);//不判断返回值了
   return (0==rc) ? 0 : -1;
}

#endif

