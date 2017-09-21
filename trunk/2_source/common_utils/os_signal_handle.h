#ifndef __MXX_OS_SIG_HANDLE_H_
#define __MXX_OS_SIG_HANDLE_H_

//定义signal事件处理函数原型;
typedef void Sigfunc(int); //定义函数

//功能:设置信号处理函数; signo-信号; func-处理函数指针;
//返回值:
//   源处理函数指针; SIG_ERR-表示失败;
Sigfunc * os_set_signal(int signo, Sigfunc *func);

//功能: SIGCHLD信号处理函数; 
//注: 防止子进程僵死
void os_sig_chld(int signo);

//功能: SIGPIPE信号处理函数
//注:
//   进程向已经收到RST信号的套接字执行写操作,内核向该进程发送一个SIGPIPE信号;
//   SIGPIPE默认行为是终止进程;
void os_sig_pipe(int signo);

//功能: SIGTERM信号处理函数
//注:
//   unix系统关机,init进程先给所有进程发送SIGTERM信号,等待一段时间(5~20s);
//   然后给所有进程发送SIGKILL信号;
void os_sig_term(int signo);

#endif
