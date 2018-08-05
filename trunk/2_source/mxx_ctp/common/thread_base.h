#ifndef __MTRX_THREAD_BASE_H__
#define __MTRX_THREAD_BASE_H__

#include <pthread.h> //POSIX标准线程头文件
#include <string>
#include <string.h>

#include "os_thread.h"

//定义线程服务状态,注:不是线程状态
#define THREAD_SS_INIT      0 //初始化
#define THREAD_SS_STARTING  1 //启动中
#define THREAD_SS_RUNNING   2 //运行中
#define THREAD_SS_TERMINATE 3 //终止

/*
  该类使用方式
  静态分配方式创建该类对象,当程序退出对象作用域时,线程退退出.该情况下,只能通过new方式创建对象,
  方式一:
     继承该类,实现init(),run(),clear();
     调用start()启动即可.
  方式二:
     定义线程函数;
     创建该类对象,并将线程函数绑定到该对象(bind()函数);
     调用start()启动函数
 */
class Thread_Base
{
 public:
    Thread_Base(const char *thread_name=""); 
    virtual ~Thread_Base();

 private:
    Thread_Base(const Thread_Base &obj);//禁止拷贝构造
    Thread_Base & operator=(const Thread_Base &a);//重载赋值运算符: 禁用赋值
 public://方式一 定义接口
    virtual int init();//!< 初始化函数; 线程启动前设置
    virtual void run();//!< 线程例程函数
    virtual int clear();//!< 清理资源

    //命令: 停止服务, 必须实现; 
    virtual int terminate_service() { return 0; }

 public://方式二 定义接口
    int bind_func(void *func_ptr=NULL);
	
 public: //线程控制函数
       //启动线程,不处理与业务有关的代码
       int start_thread();

       //删除线程,强制停止线程,不处理与业务有关的代码
       int kill_thread();
       
	   int stop_thread()//线程退出时的清理函数
	   {
           terminate_service();//子进程实现,停止服务	   
	       m_thread_status=THREAD_SS_TERMINATE;
		   m_thread_id=MXX_INVALID_THREDID;
           return 0;
	   }
	   
	   //清理本线程成员;
	   int clear_thread() { return -1; }
       //线程暂停
       //int pause();
       //线程继续
       //int thread_continue();
 public:
    //@brief 获取线程名
    const char * get_thread_name() { return m_thread_name; }
    //@brief 设置线程名
    void set_thread_name(const char *thread_name) 
    { 
       memset(m_thread_name, 0, sizeof(m_thread_name));
       strncpy(m_thread_name, thread_name, sizeof(m_thread_name));
    }

    //@brief 等待线程结束
    void join();   

 private:
       pthread_t m_thread_id;
       static int m_thread_count;//!< 线程计数器
     
 protected:
    char m_thread_name[64];//!< 线程名
	int m_thread_status;//!< 线程状态
       //int m_service_status;//服务状态,注不是线程状态
};


#endif

