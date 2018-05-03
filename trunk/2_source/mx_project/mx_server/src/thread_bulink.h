/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义业务服务连接类,每个服务进程启动一个连接
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
#ifndef __MXX_BULINK_THREAD_H_
#define __MXX_BULINK_THREAD_H_

#include "thread_base.h"

#define MXX_SOCKET_INVALID_FD  -1 //!<无效socket文件描述符

//业务连接状态
//由于下层业务需要注册业务信息,故必须设置业务状态
#define LNK_STAT_INIT          0 //!< 初始状态, 线程也没有启动, 即业务并没有连接
#define LNK_STAT_NONE_SERVICE  1 //!< 线程空运行,没有业务连接; 如服务进程突然崩溃
#define LNK_STAT_LINKING       2 //!< 连接中,一般注册信息
#define LNK_STAT_IDLE          3 //!< 服务空间,可以分配任务
#define LNK_STAT_HIGH_LOAD     4 //!< 高负载, 正忙着呢,不要烦我
#define LNK_STAT_TERMINATE     5 //!< 已经关闭,有人要求我停止服务; 禁止关联业务服务

//定义业务服务线程: 每个线程关联一个业务服务进程
class CBuLinkThread : public Thread_Base
{
   public:
     CBuLinkThread(const char *thread_name="bulisten_thread");
     virtual ~CBuLinkThread();
   private:
     //禁用拷贝构造函数
     CBuLinkThread(CBuLinkThread &obj);
	 //禁用复制函数
   public:
     virtual int init();
     virtual void run();
     virtual int terminate_service();//!< 线程退出命令
   public:
     int loadini(char *cfgfile);
	 int bind_to_socket(int so);
   private:
     //接收例程
     int service_routine();
   private:
     //业务相关信息
     int  m_groupinfo_index;//!< 该服务组(支持业务)信息描述索引; 本服务产生
     char m_group_no[64];   //!< 进程所属, 服务组号; 底层服务注册;
	 char m_buversion[16];  //!< 业务版本信息; 底层服务注册
	 
	 //socket相关信息
     int m_sock_fd;     //!< 服务端口号
     int m_max_listen;  //!< 最大连接数
     int m_recv_timeout;//!< socket接收超时时间,单位毫秒
     int m_send_timeout;//!< socket发送超时时间,单位毫秒
     
	 //业务连接状态
	 int m_link_stat; //!< 服务状态,见LNK_STAT_XXX定义
     
	 //服务是否在运行
	 bool m_b_running;//!< 表明服务是否在运行; true-在run循环中;false-退出run循环;
     bool m_stop_flag;//!< 服务停止命令标记; HEARTBEAT_STOP_TRUE-收到停止命令, 服务需要退出; HEARTBEAT_STOP_FALSE-没有收到停止命令,服务可以继续运行;
};

#endif

