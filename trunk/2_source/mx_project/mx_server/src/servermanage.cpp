#include "ConfigFile.h"
#include "server_cfg_info.h"
#include "logfile.h"
#include "servermanage.h"
#include "servercontext.h"


CServerManage::CServerManage():m_heart_thread(NULL)
{
}

CServerManage::~CServerManage()
{
    stop_service();
}

//@brief 初始化服务进程上线文信息
int CServerManage::init()
{
	//初始化服务进程上线文信息...
	CServerContext *ctx_instance=CServerContext::create_instance();
	if(NULL==ctx_instance)
		return -1;
	ConfigFile cfg;
	int rc=cfg.load_cfg_file(SERVER_CFG_FILENAME);
	if(rc < 0)
	{
	    ERROR_MSG("打开配置文件失败, rc=[%d]", rc);
	    return -1;
	}

	char serve_section[]="BalanceCtrlCenter";
	int bccid =cfg.read_int(serve_section,  "bcc_id", -1);//!< 控制中心代码; 任何两个控制中心不能相同
	if(bccid<0)
	{
		ERROR_MSG("read cfg option [%s]bccid failed! rc=[%d]", serve_section, rc);
		return -2;
	}
	ctx_instance->m_bcc_id = bccid;

	return 0;

}
/**
 * @brief 启动服务
 * @note
 *    1. 启动心跳线程;
 *    2.
 **/
int CServerManage::start_service()
{
    int rc=0;

      //启动心跳线程
    rc=start_heartbeat_thread(SERVER_CFG_FILENAME);//
    if(rc<0)
        return -1;
    
    rc=start_bulisten_thread(SERVER_CFG_FILENAME);

    return 0;
}

/**
 * @brief 停止服务
 * @note
 *    1. 停止心跳线程;
 *    2.
 **/
int CServerManage::stop_service()
{
	 stop_bulisten_thread();
    stop_heartbeat_thread();
    CServerContext::delete_instance();
    return 0;
}

//@brief 启动心跳线程; 0-成功;<0-失败;
int CServerManage::start_heartbeat_thread(const char *cfgfile)
{
    if(NULL==m_heart_thread)
        m_heart_thread=new HeartBeatThread();
    if(NULL==m_heart_thread)
        return -1;
    int rc;
    rc=m_heart_thread->load_ini(cfgfile);
    if(rc<0)
    {
        ERROR_MSG("启动心跳线程失败! 加载配置文件[%s]错误, rc=[%d]", cfgfile, rc);
        return -1;
    }
    if(!m_heart_thread->enabled())
    {
        WARN_MSG("禁用心跳线程");
        return 0;
    }
    rc=m_heart_thread->start_thread();
    if(rc<0)
    {
        ERROR_MSG("启动心跳线程失败! 创建心跳线程失败, rc=[%d]", rc);
        return -2;
    }
    INFO_MSG("心跳线程启动成功");
    return 0;
}

//@brief 停止心跳线程; 0-成功;<0-失败;
int CServerManage::stop_heartbeat_thread()
{
    if(NULL==m_heart_thread) return 0;
    m_heart_thread->stop_thread();
    delete  m_heart_thread;
    m_heart_thread=NULL;
    return 0;
}

//@brief 启动bulisten服务
int CServerManage::start_bulisten_thread(const char *cfgfile)
{
	CServerContext *ctx_instance=CServerContext::get_instance();
	if(NULL==ctx_instance)
		return -1;
	int rc=ctx_instance->m_bulisten_thread.loadini(cfgfile);
	if(rc<0)
		return rc;
	rc=ctx_instance->m_bulisten_thread.start_thread();
	if(rc<0)
	{
		ERROR_MSG("启动bulistenthread failed! rc=[%d]", rc);
		return rc;
	}
	return 0;
}

//@brief 停止bulisten服务
int CServerManage::stop_bulisten_thread()
{
	CServerContext *ctx_instance=CServerContext::get_instance();
	if(NULL!=ctx_instance)
	{
		ctx_instance->stop_buthread();
	}


	return 0;
}
