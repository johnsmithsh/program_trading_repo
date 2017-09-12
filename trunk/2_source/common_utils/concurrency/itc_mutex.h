#ifndef __THREAD_MUTEX_H_
#define __THREAD_MUTEX_H_

#define __STD_POSIX_MACRO__ //暂时使用POSIX库

#ifdef __STD_POSIX_MACRO__ //使用POSIX库
#include <pthread.h>
#else //#ifdef __STD_BOOST_MACRO__ //使用BOOST库
#include "boost/thread/mutex.hpp"
#endif
/**
 * 文件
 * 说明
 *    该文件定义线程锁，用以封装不同多线程库;
 *    此处目的不是为了定义一种新的锁机制，而是为了封装已有的线程库;
 *    不同的实现方式需要通过继承该类或直接实现该类接口。
 *    例如:在thread_mutex_boost.cpp中使用boost库结成或实现接口即可;
 *           thread_mutex_posix.cpp中使用posix标准库实现等;
 *    可通过同样的方式实现进程锁.
 * 作者:
 * 修改历史:
 * 格式: 时间 日期 修改人  注释
 *
 * */

//该类用于线程锁
//这只是一个接口
//实现方式有posix、boost等多种方式
class ItcMutex
{
   public:
      ItcMutex();
      virtual ~ItcMutex();
   public:
     //加锁
     virtual int lock();
     //解锁
     virtual int unlock();
     //试图加锁
     virtual int try_lock(double milli_second=0);
   protected:
#ifdef __STD_POSIX_MACRO__
     pthread_mutex_t m_mutex;
#elif defined(__STD_BOOST_MACRO__)
     boost::mutex m_mutex;
#endif
   private:  bool m_init_succ_flag;
   protected:  bool is_init_succ() { return m_init_succ_flag; }
};


//原理:类进入作用域申请，退出作用域释放；故可用来自动加锁与解锁
//该建议使用栈分配方式，不建议使用堆分配方式(即new)
class AutoMutex
{
   public:
      AutoMutex(ItcMutex *pMutex, int mill_second=0)
      {
          m_thread_lock=pMutex;
          m_thread_lock->lock();
      }
      ~AutoMutex()
      {
          m_thread_lock->unlock();
      }
      
      friend int operator==(const AutoMutex &obj1, const AutoMutex &obj2);
   private:
      ItcMutex *m_thread_lock;

   private://该函数禁止赋值与拷贝,禁止无参数构造
      //禁止无参数构造函数
      AutoMutex(){}
      //禁止拷贝构造
      AutoMutex(const AutoMutex &obj);// { return *this;}
      //禁止值拷贝
      AutoMutex & operator=(const AutoMutex &obj);// {}
     
};

inline int operator==(const AutoMutex &obj1, const AutoMutex &obj2)
{
   return (obj1.m_thread_lock==obj2.m_thread_lock) ? 1 : 0;
}

#endif
