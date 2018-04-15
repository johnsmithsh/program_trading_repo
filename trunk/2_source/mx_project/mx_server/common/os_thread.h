#ifndef __OS_THREAD_H__
#define __OS_THREAD_H__
/*
 * 说明: 仿造python的os.path模块定义c语言常用的路径操作
 *
 * */

#ifdef _WIN32
    #include <windows.h>
	typedef HANDLE THREAD_HANDLE;
	typedef DWORD  THREAD_ID;
    //#pragma warning(disable: 4786)
#else
    #include <pthread.h>
	#include <sys/types.h>
    #include <unistd.h>
	typedef pthread_t THREAD_HANDLE;
	typedef pthread_t THREAD_ID;
	
	typedef pid_t PID_T;
#endif

#ifdef __cplusplus
extern "C" {
#endif

//功能: 当前线程id
inline THREAD_ID get_current_threadid()
{
#ifdef WIN32
	return ::GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

//@brief 功能:当前进程id
inline PID_T  os_getpid()
{
    return getpid();
}

//@brief 功能: 获取父进程id
inline PID_T os_getppid()
{
    return getppid();
}


//@brief功能: 当前线程休眠,单位:毫秒; 范围[0,10000]=[0,10秒]
//@note 不保证一定会休眠指定时间; 可被signal中断
void os_thread_msleep(unsigned int millisecond);

#ifdef __cplusplus
}
#endif

#endif
