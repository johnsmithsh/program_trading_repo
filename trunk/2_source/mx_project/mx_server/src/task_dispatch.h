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
#ifndef _MXX_TASK_DISPATCH_H_
#define _MXX_TASK_DISPATCH_H_

#include <vector>
#include <list>
#include <queue>
//#include <deque>
#include "taskinfo.h"

//定义任务分发器
class CTaskDispatch
{
  public:
	 //@brief 向业务线程分配任务; 0-成功; <0-失败;
    int dispatch_task(CTaskSession * task_session);

    //@brief (业务线程)任务已经完成
    int task_done(CTaskSession * task_session);

    //@brief 获取等待处理任务
    int fetch_wait_task(std::vector<CTaskSession *> task_vec);

    //@brief 获取已处理完成的队列
    int fetch_done_task(std::vector<CTaskSession *> task_vec);
  private:
    std::queue<CTaskSession *>  m_task_wait_que;    //!< 等待处理; 请求处理必须采用时间有限,故采用队列;
    std::list<CTaskSession *>   m_task_process_que; //!< 正在处理队列; 为了处理完成后方便查询,故只能使用链表
    std::queue<CTaskSession *>  m_task_done_que;    //!< 已完成队列,等待发送;

};
#endif
