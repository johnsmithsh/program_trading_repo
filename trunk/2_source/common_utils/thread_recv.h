#ifndef __MXX_RECV_THREAD_H_
#define __MXX_RECV_THREAD_H_

#include "thread_base.h"
#include "mxx_bin_pack.h"

class CRecvThread : public Thread_Base
{
   public:
     CRecvThread(const char *thread_name="recv_thread");
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


/////////////////////////////////////////////////////////////////////
//操作(接收)缓存队列

//队列尾部增加
int recv_que_push(ST_BIN_BUFF *bin_buff);

//队列头获取
ST_BIN_BUFF * recv_que_pop();

//////////////////////////////////////////////////////////////////////
#endif
