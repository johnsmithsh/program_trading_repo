
#include <string.h>

#include "thread_base.h"

int Thread_Base::m_thread_count=0;

void* thread_proc_posix(void *arg_ptr)
{
    if(NULL == arg_ptr)
        return NULL;
    Thread_Base *obj_ptr=(Thread_Base *)arg_ptr;
    obj_ptr->init();
    obj_ptr->run();
    obj_ptr->clear();

    return NULL;
}

//Thread_Base::Thread_Base()
//{
//    memset(&m_thread_id, 0, sizeof(m_thread_id));
//}

Thread_Base::Thread_Base(const char *thread_name/*=""*/)
{
    m_thread_id=0;
    memset(m_thread_name, 0, sizeof(m_thread_name));
    if(NULL!=thread_name)
      strncpy(m_thread_name, thread_name, sizeof(m_thread_name));
}

Thread_Base::~Thread_Base()
{
}

//初始化
int Thread_Base::init()
{
    return 0;
}

void Thread_Base::run()
{
   return;
}

//清理资源
int Thread_Base::clear()
{
    return 0;
}

//创建线程
int Thread_Base::start()
{
    int rc;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    //设置线程竞争范围
    // PTHREAD_SCOPE_SYSTEM与整个系统中的线程竞争CPU
    // PTHREAD_SCOPE_PROCESS与进程中的线程竞争CPU,
    // POSIX尚未实现PTHREAD_SCOPE_PROCESS
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    //pthread_attr_setstacksize(&attr, 1000);//设置线程栈大小

    //设置线程调度策略,目前POSIX仅实现SCHED_OTHER
    //pthread_attr_setschedpolicy(&attr, SCHED_FIFO/SCHED_RR/SCHED_OTHER)

    //线程优先级只有在调度策略是SCHED_FIFO/SCHED_RR时才有效
    //phtread_attr_setschedparam()&attr, 0;//设置线程优先级

    rc=pthread_create(&m_thread_id, &attr, thread_proc_posix, this);
    if(0!=rc)
        return -1;

    pthread_attr_destroy(&attr);

    return 0;
}

int Thread_Base::kill()
{
   return 0;
}
