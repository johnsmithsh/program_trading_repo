#include "ConfigFile.h"
#include "logfile.h"

#include "server_cfg_info.h"
#include "servermanage.h"
#include "servercontext.h"


CServerManage::CServerManage()
{
}

CServerManage::~CServerManage()
{
    stop_service();
    
    CServerContext::delete_instance();
}

//@brief 初始化服务进程上线文信息
int CServerManage::init()
{
	//初始化服务进程上线文信息...
	CServerContext *ctx_instance=CServerContext::create_instance();
	if(NULL==ctx_instance)
		return -1;
	int rc=ctx_instance->load_ini(SERVER_CFG_FILENAME);
    if(rc<0)
    {
        ERROR_MSG("加载进程上下文信息失败");
        return -1;
    }

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

    CServerContext *ctx_instance=CServerContext::get_instance();
	if(NULL==ctx_instance)
		return -1;
    int rc=ctx_instance->m_bu_thread.load_ini(CONFIG_FILENAME);
    if(rc<0)
    {
        ERROR_MSG("业务线程加载配置信息失败");
        return -2;
    }
    ctx_instance->m_bu_thread.start_thread();

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
	CServerContext *ctx_instance=CServerContext::create_instance();
	if(NULL==ctx_instance)
		return -1;

    ctx_instance->m_bu_thread.stop_thread();
    
    
    return 0;
}
