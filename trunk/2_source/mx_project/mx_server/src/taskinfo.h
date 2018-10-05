/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义任务分发器,将请求分发给不同的业务进程
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
#ifndef _MXX_TASK_INFO_H_
#define _MXX_TASK_INFO_H_

#include <stdint.h>
#include <vector>
#include "mxx_mem.h"

#define TIMESTAMP_RECV   '1' //!<时间戳: (第一个)接收时间
#define TIMESTAMP_ASSIGN '2' //!<时间戳: 分配任务时间
#define TIMESTAMP_DONE   '3' //!<时间戳: 任务完成时间
#define TIMESTAMP_SEND   '4' //!<时间戳: (最后一个)应答发送时间

//@brief 统计每个请求的处理时间
typedef struct __task_time_t_
{
    struct timespec tm_recv;    //!< 接收任务时间
    struct timespec tm_buassign;//!< 分配任务时间
    struct timespec tm_budone;  //!< 任务处理完成时间
    struct timespec tm_send;     //!< 任务应答时间
}task_time_t;

//@brief 释放内存函数
typedef void (*PF_MEM_FREE)(void *);

//定义任务消息,外部的每次请求就是一个任务
//一个任务包含接收socket、多个请求、多个应答、统计信息、
class CTaskSession
{
  public:
     CTaskSession();
     virtual ~CTaskSession();
  private:
     CTaskSession& operator=(const CTaskSession &obj);
     CTaskSession(const CTaskSession &obj);
  public:
     //@brief 清空数据
     void clear();


     //void push_req_frame(ST_SOCK_FRAME*sock_frame);
     //void push_rsp_frame(ST_SOCK_FRAME*sock_frame);

     //@brief 添加一个请求数据包
     void push_req_frame(mxx_varmem_t *frame);
     //@brief 添加一个应答数据包
     void push_rsp_frame(mxx_varmem_t *frame);

     //@brief 设置socket
     void set_socket(int so)                 { m_recv_so = so; }

     //void set_request_id(uint64_t serial_no) { m_nLocalRequestId=serial_no; }

     //@brief 更新任务时间; tiemstamp_type定义见TIMESTAMP_XXX
      void update_tiemstamp(char timestamp_type);
  public:
    int                 m_recv_so;        //!< 接收任务的socket;
    std::vector<void *> m_reqFrameVec;    //!< 请求数据包
    std::vector<void *> m_rspFrameVec;    //!< 应答消息包
    uint64_t            m_nLocalRequestId;//!< 请求id, 控制中心向业务进行分发任务时,生成该id,应答时回填该id; 用于匹配对应的请求;
    task_time_t         m_taskTimeStamp;  //!< 时间戳
  //private:
  //  static PF_MEM_FREE pf_free;//!< 释放内存指针
  private:
    //@brief 返回下一个序号
    static uint64_t next_task_serial();
    static uint64_t m_current_serial; //!<任务流水好
};
#endif
