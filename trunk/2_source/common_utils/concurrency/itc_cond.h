#ifndef __MXX_ITC_COND_H_
#define __MXX_ITC_COND_H_H

/*
 *
 * 封装条件变量;
 * 条件变量通常与互斥量一起使用;
 *    一个时间发生时需要发送信号,此时可能有多个进行或线程正在等待该信号; 条件变量主要用于这种情况;
 *    条件变量和互斥量共同使用; 一般情况下,如果使用条件变量,则必须同一个互斥量关联;
 *    互斥量操作:初始化init/销毁destroy/等待wait/超时等待timedwait/发信号signal/广播broadcast
 * pthread_cond_wait内部会解锁，然后等待条件变量被其它线程激活;
 * 不能由多个线程同时初始化一个条件变量.当需要重新初始化或释放一个条件变量时,应用程序必须保证这个条件变量未被使用;
 * 唤醒丢失问题:
 *      在线程未获得相应的互斥量时调用pthread_cond_signal或pthread_cond_broadcast可能会引起唤醒丢失;
 *      会引起如下问题:
 *          一个线程调用phtread_cond_signal或pthread_cond_broadcast,另一线程正在测试条件变量和调用phtread_cond_wait之间;
 *          没有线程处于阻塞或等待状态;
 *
 *
 * 使用说明:
 *    由于条件变量的特殊性,该封装中存在一个mutex+cond;
 *    1.wait流程:
 *        ItcCond::lock()成功->检查是否需要等待->ItcCond::wait()/ItcCond::trywaiwt()->条件处理(尽量简短,不要太耗时)->ItcCond::unlock();
 *    2.signal/broadcast流程:
 *        ItcCond::lock()成功;->流程处理->ItcCond::signal/ItcCond::broadcast->ItcCond::unlock();
 * */
#include <pthread.h>

#define MXX_COND_SUCC                    (0)   //成功                  
#define MXX_COND_FAILED                 (-1)   //没有收到条件变量

//封装条件变量
class ItcCond
{
   public:
       ItcCond();
       ItcCond(int shared);
       virtual ~ItcCond();
      
       int lock(); 
       int unlock();
       int wait();
       int try_wait(double milli_second=0.0);
       
       int signal();
       int broadcast();

   private:
      //禁用拷贝构造函数
      ItcCond(const ItcCond &obj);
      //禁用赋值运算符
      ItcCond & operator=(ItcCond &obj);
   private:
     pthread_cond_t m_cond;//条件变量
     //pthread_cond_attr_t m_cond_attr;//条件变量属性 linux pthread没有实现cond_attr
     pthread_mutex_t m_mutex;//互斥量,用于保护条件变量
     bool b_init_succ_flag;//true-初始化成功; false-初始化失败;
   private:
      //功能: 判断信号量是否初始化成功;
      bool is_init_succ() { return b_init_succ_flag;}
      //功能:初始化条件变量; shared:0-(进程内)线程共享; 1-(父子)进程间信号量;
      //返回值: 0-成功; <0-失败;
      int init(int shared);
};

#endif

