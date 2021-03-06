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
 *      4. sem_wait/sem_trywait/sem_timedwait可被signal中断或超时中断;
 *      注:测试环境centos 6.8 + vmware 7.1 + gcc4.4.7
 *
 *
 ************************************************************************************/


#include <semaphore.h>

#define MXX_SEM_SUCC                    (0)   //成功
#define MXX_SEM_FAILED                 (-1)   //没有获取信号量
#define MXX_SEM_ERROR_INIT_FAILED      (-2)   //sem初始化失败
#define MXX_SEM_ERROR_WAIT_SIG_INTERR  (-3)   //sem wait 被signal中断
#define MXX_SEM_ERROR_WAIT_TIMEOUT     (-4)   //sem wait等待超时
#define MXX_SEM_ERROR_NO_SEM_COMMING   (-5)   //没有等到需要的信号

//为了便于使用,封装semaphore
class ItcSem
{
   public:
      //构造函数
      ItcSem();
      //功能:构造函数; shared:0-(进程内)线程共享; 1-(父子)进程间信号量; init_value:信号量初始值;
      ItcSem(int shared, int init_value);

      //析构函数
      virtual ~ItcSem();

      //功能:递减信号量,即等待事件; 返回值:-1-没有等到信号量; 0-等到需要的信号量;
      int wait();
      //功能:递减信号量(即等待事件); milli_second:超时事件,单位:毫秒; 返回值: 0-等到需要的信号量; <0-没有等到需要的信号量;
      int try_wait(double milli_second=0.0);

      //功能: 信号量递增, 即发出事件通知; 0-成功; <0失败;
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
     bool b_init_succ_flag;//true-初始化成功; false-初始化失败;
   private:
      //功能: 判断信号量是否初始化成功;
      bool is_init_succ() { return b_init_succ_flag;}
      //功能:初始化信号量; shared:0-(进程内)线程共享; 1-(父子)进程间信号量; init_value:信号量初始值;
      //返回值: 0-成功; <0-失败;
      int init(int shared, int init_value);
};
#endif
