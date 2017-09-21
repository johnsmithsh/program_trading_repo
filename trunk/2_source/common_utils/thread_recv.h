#ifndef __MXX_RECV_THREAD_H_
#define __MXX_RECV_THREAD_H_

#include "thread_base.h"

class CRecvThread : public Thread_Base
{
   public:
     CRecvThread(char *thread_name="recv_thread");
     virtual ~CRecvThread();
   private:
     //禁用拷贝构造函数
     CRecvThread(CRecvThread *obj);
   public:
     virtual int init();
     virtual void run() {  recv_routine(); }
     virtual int terminate();

   private:
     //接收例程
     int recv_routine();
   private:
     int m_server_port;//服务端口号
};

#endif
