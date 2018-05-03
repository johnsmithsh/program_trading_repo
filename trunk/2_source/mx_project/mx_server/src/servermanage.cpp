
#include "logfile.h"
#include "servermanage.h"


CServerManage::CServerManage():m_heart_thread(NULL)
{
}

CServerManage::~CServerManage()
{
    stop_service();
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
    rc=start_heartbeat_thread("./conf/master.ini");//
    if(rc<0)
        return -1;
    
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
    stop_heartbeat_thread();
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
