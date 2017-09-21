/*
 *
 * linux进程和线程执行过程中会被signal中断,故需要正确处理这些信号,否则会引起僵死进程等一类的问题
 * linux配置signal有来各种方式,一种使用signal函数,另一种使用sigaction函数;
 * signal早于POSIX标准出现,为了向后兼容,不同的实现有不同的信号语义;
 * POSIX明确规定了sigaction的语义,故为了保持源代码的可移植性,我们使用sigaction.
 * */

#include "os_signal_handle.h"

#include <signal.h>
#include <sys/wait.h>

//功能:设置信号处理函数; signo-信号; func-处理函数指针; 
//返回值:
//   源处理函数指针; SIG_ERR-表示失败;
Sigfunc * os_set_signal(int signo, Sigfunc *func)
{
   struct sigaction act, old_act;
   act.sa_handler=func;
   sigemptyset(&act.sa_mask);//sa_mask置空,意味着该信号处理函数运行期间,不阻塞额外的信号.
   if(SIGALRM == signo)
   {
 #ifdef SA_INTERRUPT
      act.sa_flags |= SA_INTERRUPT;/* SunOs 4.x */
 #endif    
   }
   else 
   {
 #ifdef SA_RESTART
      act.sa_flags |= SA_RESTART;/* SVR4, 4.4BSD */
 #endif
   }
   if(sigaction(signo, &act, &old_act)<0)
   {
      return (SIG_ERR);
   }
   return (old_act.sa_handler);
}

//功能: SIGCHLD信号处理函数;
//
//注:
//   如果不处理SIGCHLD,子进程退出后一直处于僵死状态,占用系统资源;
//   为防止多个子进程同时退出而导致的SIGCHLD信号丢失,建议使用waitpid;
//   虽然在systemV和unix 98实现中,SIGCHLD设置为SIG_IGN后子进程不会进入僵死,但是POSIX没有指定这种标准;
void os_sig_chld(int signo)
{
   pid_t pid;
   int stat;

   //unix信号不排队,且同时收到多个SIGCHLD时信号函数仅执行一次;
   //故使用waitpid+WNOHANG;
   while((pid=waitpid(-1,&stat, WNOHANG))>0)
       ;//尚未想好收到SIGCHILD后该怎么处理;但不建议在信号处理函数中调用标准I/O函数;

   return;
}

//功能: SIGPIPE信号处理函数
//注:
//   进程向已经收到RST信号的套接字执行写操作,内核向该进程发送一个SIGPIPE信号;
//   SIGPIPE默认行为是终止进程;
void os_sig_pipe(int signo)
{
}

//功能: SIGTERM信号处理函数
//注:
//   unix系统关机,init进程先给所有进程发送SIGTERM信号,等待一段时间(5~20s);(SIGTERM默认处理方式是终止进程,可以忽略该信号)
//   然后给所有进程发送SIGKILL信号;
//   故必须在客户中使用select或poll,服务器终止,则客户就能检测到;
void os_sig_term(int signo);
{
}
