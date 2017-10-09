#ifndef _CTRL_CENTER_THREAD_H_
#define _CTRL_CENTER_THREAD_H_

#include "thread_base.h"

//控制中心
class CCtrlCenterThread : public Thread_Base
{
   public:
     CCtrlCenterThread(const char *thread_name="ctrl_center_thread");
     virtual ~CCtrlCenterThread();
   private:
     //禁用拷贝构造函数
     CCtrlCenterThread(CCtrlCenterThread *obj);
   public:
     virtual int init();
     virtual void run() {  handle_routine(); }
     virtual int terminate();

   private:
     //接收例程
     int handle_routine();
   //private:
   //  int m_server_port;//服务端口号
};

#endif
