#ifndef __MTRX_THREAD_BASE_H__
#define __MTRX_THREAD_BASE_H__

#include <pthread.h> //POSIX标准线程头文件
#include <string>
#include <string.h>

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
 public://方式一
    virtual int init();//初始化函数
    virtual void run();//线程例程函数
    virtual int clear();//清理资源

    virtual int terminate()//线程退出时的清理函数
    { return 0;}
    //停止线程,包含业务有关的代码
    virtual int stop()
    { return 0; }

 public://方式二
    int bind_func(void *func_ptr=NULL);
 public: //线程控制函数
       //启动线程,不处理与业务有关的代码
       int start();

       //删除线程,强制停止线程,不处理与业务有关的代码
       int kill();
       
       //线程暂停
       //int pause();
       //线程继续
       //int thread_continue();
 public:
    //获取线程名
    std::string get_thread_name();
    //设置线程名
    void set_thread_name(const char *thread_name) 
    { 
       memset(m_thread_name, 0, sizeof(m_thread_name));
       strncpy(m_thread_name, thread_name, sizeof(m_thread_name));
    }

    //等待线程结束
    void join();   

 private:
       pthread_t m_thread_id;
       static int m_thread_count;//线程计数器
 protected:
    char m_thread_name[48];//线程名
       //int m_service_status;//服务状态,注不是线程状态
};


#endif

