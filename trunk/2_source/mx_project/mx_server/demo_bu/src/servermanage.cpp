#include "ConfigFile.h"
#include "logfile.h"
#include "servermanage.h"
#include "servercontext.h"


CServerManage::CServerManage()
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
