#ifndef __MXX_OS_SIG_HANDLE_H_
#define __MXX_OS_SIG_HANDLE_H_

//定义signal事件处理函数原型;
typedef void Sigfunc(int); //定义函数

//功能:设置信号处理函数; signo-信号; func-处理函数指针;
//返回值:
//   源处理函数指针; SIG_ERR-表示失败;
Sigfunc * os_set_signal(int signo, Sigfunc *func);

//功能: SIGCHLD信号处理函数;
void os_sig_chld(int signo);

#endif
