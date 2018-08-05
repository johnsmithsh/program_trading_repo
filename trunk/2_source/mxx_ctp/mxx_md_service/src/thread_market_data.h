/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    处理收到的行情
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
#ifndef __MXX_THREAD_MARKETDATA_H_
#define __MXX_THREAD_MARKETDATA_H_

#include <deque>
#include "thread_base.h"
#include "itc_mutex.h"
#include "ctp_md.h"


typedef  CThostFtdcDepthMarketDataField ST_MARKET_DATA;

//行情分发器(线程):即处理收到的行情
class CMarketDataDistributor : public Thread_Base
{
   public:
     CMarketDataDistributor(const char *thread_name="marketdataDistributor_thread");
     virtual ~CMarketDataDistributor();
   private:
     //禁用拷贝构造函数
     CMarketDataDistributor(CMarketDataDistributor &obj);
	 //禁用复制函数
     CMarketDataDistributor &operator=(CMarketDataDistributor &obj);
   public:
     virtual int init();
     virtual void run();
     virtual int terminate_service();//!< 线程退出命令
   public:
     int loadini(char *cfgfile);
   public://对外接口
     //处理收到的行情
     void process_market_data(ST_MARKET_DATA *md);
     void add_market_data(ST_MARKET_DATA &md);
   
   private://辅助业务处理函数
     int service_routine();
     
     //行情消息推送到客户端
     //即:放入对应的客户端发送线程的缓存即可;
     int push_msg_to_client();
   private:
       ItcMutex m_mutex;//!< 用于控制send_request_to_bu,防止同时调用该接口函数
   private:
	 bool m_b_running;//!< 表明服务是否在运行; true-在run循环中;false-退出run循环;
     bool m_stop_flag;//!< 服务停止命令标记; HEARTBEAT_STOP_TRUE-收到停止命令, 服务需要退出; HEARTBEAT_STOP_FALSE-没有收到停止命令,服务可以继续运行;
	 
	 //ST_BUTHREAD_STATIC m_static;//!< 统计时间
  private:
     std::deque<ST_MARKET_DATA> m_md_que;//!< 行情队列,收到行情后,存入该队列
	 
};

#endif

