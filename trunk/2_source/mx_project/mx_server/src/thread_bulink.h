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

#include <deque>

#include "thread_base.h"
#include "msg_link_define.h"
#include "itc_mutex.h"
#include "taskinfo.h"
#include "svr_link.h"

#define MXX_SOCKET_INVALID_FD  -1 //!< 无效socket文件描述符

//业务连接状态
//由于下层业务需要注册业务信息,故必须设置业务状态
#define LNK_STAT_INIT          0 //!< 初始状态, 线程也没有启动, 即业务并没有连接
#define LNK_STAT_NONE_SERVICE  1 //!< 线程空运行,没有业务连接(即socket); 如服务进程突然崩溃
#define LNK_STAT_LINKING       2 //!< 连接中,一般注册信息
#define LNK_STAT_REGISTERING   3 //!< 等待bu注册
#define LNK_STAT_READY         4 //!< 服务空闲,可以分配任务
#define LNK_STAT_HIGH_LOAD     5 //!< 高负载, 正忙着呢,不要烦我
#define LNK_STAT_DISCONNING    6 //!< 正在关闭连接
#define LNK_STAT_TERMINATE     7 //!< 已经关闭,有人要求我停止服务; 禁止关联业务服务

typedef struct __st_bulink_static
{
    time_t start_time;
	unsigned long req_count;//!< 请求计数
	unsigned long elap_max; //!< 最大处理时间
	unsigned long elap_min; //!< 最小处理时间
	unsigned long total_time;//!< 总处理业务时间
}ST_BUTHREAD_STATIC;

//定义业务服务线程: 每个线程关联一个业务服务进程
class CBuLinkThread : public Thread_Base
{
   public:
     CBuLinkThread(const char *thread_name="bu_thread");
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
     bool check_can_bindsocket() { return (LNK_STAT_INIT==m_link_stat)||(LNK_STAT_NONE_SERVICE==m_link_stat); }
     int bind_to_socket(int so);
     void get_bulinkinfo(ST_SVR_LINK_HANDLE &bulinkinfo);
   public://对外接口
     //@brief (向该线程)分配任务;
	 // 注: 处理业务数据就不适用回调函数了,处理结果放入到应答队列,调用者自取处理结果;
     int send_request_to_bu(unsigned char *data, size_t data_len);
     int send_request_to_bu(CTaskSession * task_session=NULL);
   
       //@返回业务链接状态
     int get_link_stat() { return m_link_stat; }
   private:
       //接收例程; 处理来自业务进程的消息
     int service_routine();
     //@brief 处理socket关闭的情况
     void process_socket_close();
   private://业务处理函数
    int wait_buconn();
	int wait_buregister();
	//处理来自业务进程消息,每次处理一个;
	int process_msg();
	//@brief [连接请求]控制中心处理业务进程发起连接请求;
    int process_bulink(ST_MSGLINK_BUFF *msg_buff_ptr);
	//@brief [断开连接](业务进程|控制中心)发起断开连接请求;
    int process_disconn(ST_MSGLINK_BUFF *msg_buff_ptr);
	//@brief [注册业务]控制中心处理业务进程发起注册请求;
	int process_buregister(ST_MSGLINK_BUFF * msg_buff_ptr);
	
	//@brief 控制中心处理业务进程的应答数据;
    //注: 业务进程处理业务请求后,将处理结果返回;
    int process_buresponse(ST_MSGLINK_BUFF * msg_buff_ptr);
	//@brief [数据传输]控制中心处理业务进程的传递数据;
    //注: 业务进程处理业务请求后,将处理结果返回
    int process_butransfer(ST_MSGLINK_BUFF * msg_buff_ptr);
	//@brief [数据传输]控制中心处理业务进程的推送数据;
    //注: 业务进程处理业务请求后,将处理结果返回
    int process_bupush(ST_MSGLINK_BUFF * msg_buff_ptr);
   
   private://辅助业务处理函数
    //发送应答数据
    int send_responsedata(){ return -1; }
	//发送推送数据
	int send_pushdata(){ return -1; }
	//发送数据
	int send_transferdata(){ return -1; }
   private://辅助函数
      int check_msg_info(ST_MSGLINK_BUFF *msg_buff_ptr, unsigned short msgid);
	   int clear_buinfo();
	   int clear_sockinfo();

	   int recv_msg(ST_MSGLINK_BUFF *msg_buff);
	   int send_msg(ST_MSGLINK_BUFF *msg_buff);
   private:
       ItcMutex m_mutex;//!< 用于控制send_request_to_bu,防止同时调用该接口函数
   private:
     //业务进程相关信息
     int  m_group_id;//!< 该服务组(支持业务)信息描述索引; 本服务产生
     char m_group_no[64];   //!< 进程所属, 服务组号; 底层服务注册;
     char m_buversion[16];  //!< 业务版本信息; 底层服务注册
     char m_buprogname[64]; //!< 另一端业务进程的程序名
     char m_bu_pid[32];     //!< 业务进程id
     int  m_bu_no;//!< 控制中心分配给业务进程的业务号
	 
	 //socket相关信息
     int m_sock_fd;     //!< connection socket
     //int m_max_listen;  //!< 最大连接数
     int m_recv_timeout;//!< socket接收超时时间,单位毫秒
     int m_send_timeout;//!< socket发送超时时间,单位毫秒

     //bu连接信息
     ST_SVR_LINK_HANDLE m_link_handle;

	 //业务连接状态
	 int m_link_stat; //!< 服务状态,见LNK_STAT_XXX定义
     
	 //服务是否在运行
	 bool m_b_running;//!< 表明服务是否在运行; true-在run循环中;false-退出run循环;
    bool m_stop_flag;//!< 服务停止命令标记; HEARTBEAT_STOP_TRUE-收到停止命令, 服务需要退出; HEARTBEAT_STOP_FALSE-没有收到停止命令,服务可以继续运行;
	 
	 ST_BUTHREAD_STATIC m_static;//!< 统计时间

   private:
     std::deque<CTaskSession *> m_req_que;//!< 任务请求列表
	 
};

#endif

