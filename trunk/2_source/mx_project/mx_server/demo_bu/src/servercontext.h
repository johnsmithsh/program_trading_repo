#ifndef _MXX_SERVER_CONTEXT_H_
#define _MXX_SERVER_CONTEXT_H_

/*
 * @file
 * @desc
 *    服务进程上下文
 * */
#include "thread_bu.h"

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
      load_ini(const char *cfg_file);
  public:
      unsigned int m_bcc_id;   //!< 本控制中心的id
      unsigned int m_bcc_no;
      char m_group_no[64];
      char m_group_desc[128];

  //------------------------------------------------------------------------------------------------------
  public: //group相关操作
     CBuThread  m_bu_thread;//!< 业务处理线程

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
