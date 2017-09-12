#ifndef __MXX_ITC_SEMAPHORE_H_
#define __MXX_ITC_SEMAPHORE_H_

/**************************************************************************************
 *
 * 说明:
 *    为了便于使用,封装了POSIX标准的信号量
 *
 *  POSIX(内存)信号量说明:
 *      1. POSIX(内存)信号量使用方式是sem_init()->sem_wait()->sem_post()->sem_destroy()
 *         sem_open()/sem_unlink()/sem_close()是针对POSIX(文件)信号量的操作,主要用于IPC
 *      2. sem_init(*,0,1)初始化之后可以进行正常操作;
 *         sem_destroy删除信号量之后,sem_getvalue()、sem_wait、sem_post的返回值是0,而不是-1!!!!!,但是errno包含错误码,指定无效;
 *         sem_destroy之后,信号量竟然可以进行正常的sem_wai、sem_post,而且会阻塞线程!!!;
 *                     sem_getvalue查询到信号量的值在sem_wait、sem_post之后会进行相应的变化(虽然errno指示错误)!!!!
 *      3. sem_init(*,0,0)初始化之后,sem_getvalue虽然能查询信号量值,返回值0,但是errno==22竟然指示信号量无效；
 *                      sem_wait、sem_post能正常工作,但是依然存在errno=22的问题;
 *      注:测试环境centos 6.8 + vmware 7.1 + gcc4.4.7
 *
 *
 ************************************************************************************/


#include <semaphore.h>

//为了便于使用,封装semaphore
class ItcSem
{
   public:
      //构造函数
      ItcSem();
      ItcSem(int shared, int init_value);

      //析构函数
      virtual ~ItcSem();

      int wait();
      int try_wait(int milli_second);

      int post();
       
      //功能:返回当前信号量的值,该接口用于测试
      int get_value();
   protected:
      //禁止拷贝构造函数
      ItcSem(ItcSem &ojb);
      //禁止赋值运算符;
      ItcSem & operator=(const ItcSem &obj);
   private:
     sem_t m_sem;
     bool b_init_succ_flag;
   private:
      boo is_init_succ() { return b_init_succ_flag;}
      int init(int shared, int init_value);
 
};
#endif
