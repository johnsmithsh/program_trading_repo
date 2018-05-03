#ifndef __MXX_RECV_THREAD_H_
#define __MXX_RECV_THREAD_H_

#include "thread_base.h"
//#include "mxx_bin_pack.h"

class CRecvThread : public Thread_Base
{
   public:
     CRecvThread(const char *thread_name="recv_thread");
     virtual ~CRecvThread();
   private:
     //禁用拷贝构造函数
     CRecvThread(CRecvThread &obj);
   public:
     virtual int init();
     virtual void run() {  recv_routine(); }
     virtual int terminate_service();//!< 线程退出命令
   public:
     int loadini(char *cfgfile);
   private:
     //接收例程
     int recv_routine();
   private:
     int m_lstn_port; //<! 服务端口号
     int m_max_listen;  //<! 最大连接数
     int m_recv_timeout;//<! socket接收超时时间,单位毫秒
     int m_send_timeout;//<! socket发送超时时间,单位毫秒
     
     bool m_b_running;//!< 表明服务是否在运行; true-在run循环中;false-退出run循环;
     char m_stop_flag;//!< 服务停止命令标记; HEARTBEAT_STOP_TRUE-收到停止命令, 服务需要退出; HEARTBEAT_STOP_FALSE-没有收到停止命令,服务可以继续运行;
};


/////////////////////////////////////////////////////////////////////
//操作(接收)缓存队列

//队列尾部增加
//int recv_que_push(ST_BIN_BUFF *bin_buff);

//队列头获取
//ST_BIN_BUFF * recv_que_pop();

//////////////////////////////////////////////////////////////////////
#endif
