#ifndef _MXX_SERVER_CONTEXT_H_
#define _MXX_SERVER_CONTEXT_H_

/*
 * @file
 * @desc
 *    描述服务上下文; 该服务下的线程可通过该上下文进行交互; 单实例模式; 整个服务进程只有一个实例;
 * @detail\
 *     包含如下部分信息:
 *     1. bcc信息和业务服务序列号;
 *        2. 业务组及业务线程链接信息;
 *        3. 单实例模式操作;
 * */
#include "thread_bulink.h"
#include "thread_bulisten.h"
#include "bufuncmanage.h"

//定义服务上下文,采用单实例模式
class CServerContext
{
  private:
      CServerContext();
	  CServerContext(const CServerContext& obj);
	  CServerContext& operator=(const CServerContext& obj);

  public:
	  virtual ~CServerContext();

  public:
      //该变量组,与上级连接
      //CRecvThread m_recv_threads[10];//!< 接收线程,最多启动10个,每个线程可以监听多个连接;
	  //CForwardThread m_forward_thread[10];//!< 转发线程, 当前系统不支持,转发给其他系统
      //CPushThread    m_push_thread[10];//!< 推送线程, 将处理结果返回客户端

  public:
      unsigned int m_bcc_id;   //!< 本控制中心的id
      unsigned int m_max_buno; //!< 业务进程id分配序列,链接到控制中心时由控制中心生成并返回;
  public:
      unsigned int get_bcc_id()    { return m_bcc_id; }
      unsigned int get_next_buno() {  return ++m_max_buno; }
  //------------------------------------------------------------------------------------------------------
  //组相关信息
  public: //group相关操作
      CBuGroupInfo *find_groupinfo(char *group_no);
      void find_groupinfo(unsigned int bu_func_id, std::list<CBuGroupInfo*> &group_list);
  public: //group相关变量
      ////该变量组,与下级连接
      //int m_bu_group;
	  //CBuGroupInfo m_groupinfo[20];//!< 业务组信息,记录各业务组支持的业务功能; 每个group表示一组同类bu支持的业务
	  //
	  //CBuGroupManage m_bugroup[200];//!< 每个变量表示一个

     CBuLinkThread  m_bulink_threads[150];//!< 与每个下级系统维护一个连接,下级系统比较少
	  CBuListenThread m_bulisten_thread;   //!< 监听下级系统连接
	  //CTaskDispatch m_dispach;//!< 任务分发器,将受到的请求分发给不同的业务进程

	  //@brief bu端链接绑定到一个业务处理线程
	  int bind_socket_to_bulinkthread(int socket, char *szMsg);
	  //@brief 停止bu监听和链接线程
	  int stop_buthread();
  private:
	  ItcMutex m_bulinkthread_mutex;//!<
  //------------------------------------------------------------------------------------------------------

  public:
      static CServerContext *create_instance()
      {
    	  if(NULL!=m_instance)
    		  return m_instance;
    	  m_instance = new CServerContext();
    	  return m_instance;
      }
	  static void delete_instance()
	  {
		  if(NULL!=m_instance)
		  {
			  delete m_instance;
			  m_instance=NULL;
		  }
	  }

    static CServerContext *get_instance() { return m_instance; }
  private:
      static CServerContext* m_instance;//!< 单实例模式


};

#endif
