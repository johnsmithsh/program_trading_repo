/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义业务服务监听类,用于监听业务服务启动
 *@details 
 *Others:  
 *Function List:  
         1.………… 
         2.………… 
 *@note  
 *@history:  
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/  
#ifndef __MXX_BULISTEN_THREAD_H_
#define __MXX_BULISTEN_THREAD_H_

#include <arpa/inet.h>
#include "thread_base.h"

class CBuListenThread : public Thread_Base
{
   public:
     CBuListenThread(const char *thread_name="bulisten_thread");
     virtual ~CBuListenThread();
   private:
     //禁用拷贝构造函数
     CBuListenThread(CBuListenThread &obj);
	 //禁用复制函数
   public:
     virtual int init();
     virtual void run();
     virtual int terminate_service();//!< 线程退出命令
   public:
     int loadini(char *cfgfile);
   private:
     //接收例程
     int listen_routine();
   private:
     int m_lstn_port;   //<! 服务端口号
     int m_max_listen;  //<! 最大连接数
     int m_recv_timeout;//<! socket接收超时时间,单位毫秒
     int m_send_timeout;//<! socket发送超时时间,单位毫秒
     int m_listen_so;   //!< socket
     
     bool m_b_running;//!< 表明服务是否在运行; true-在run循环中;false-退出run循环;
     bool m_stop_flag;//!< 服务停止命令标记; HEARTBEAT_STOP_TRUE-收到停止命令, 服务需要退出; HEARTBEAT_STOP_FALSE-没有收到停止命令,服务可以继续运行;
};

#endif
