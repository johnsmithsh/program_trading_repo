/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义请求数据包; 
 *    一个业务处理过程需要多种信息: socket接收数据,业务说明信息,控制信息等;
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
#ifndef _MXX_REQ_PACK_H_
#define _MXX_REQ_PACK_H_

#include "socket_frame.h"
#include "socket_handle.h"

//请求数据包
//包括: recv收到的数据,本服务处理需要的业务信息,时间点
class CReqPack
{
  public:
      CReqPack();
	  ~CReqPack();
  public:
	  ST_SOCK_FRAME m_sock_frame;        //!< socket收到的原始数据数据
	  ST_SockConnHandle *m_socket_handle;//!< socket句柄
	  int m_recv_time;                   //!< 数据包接收时间  
};
#endif
