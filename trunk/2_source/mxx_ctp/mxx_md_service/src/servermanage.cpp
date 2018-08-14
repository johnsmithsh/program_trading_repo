
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
    //rc=start_heartbeat_thread("./conf/master.ini");//
    //if(rc<0)
    //    return -1;

    CGlobalInfo *pGlobalInfo = CGlobalInfo::create_instance();
    if(NULL==pGlobalInfo)
        return -1;
    
    rc=pGlobalInfo->load_cfg("./conf/md.ini");
    if(rc<0)
    {
        return -2;
    }
    
    pGlobalInfo->init();
    
    //连接ctp行情服务器...
    CCtpMdConnection *pCtpMd = globalinfo_get_MdConnection();
    if(NULL==pCtpMd)
    {
        return -3;
    }
    
    pCtpMd->set_autoconn_flag(true);//连接后自动登录
    rc=pCtpMd->connect();
    if(rc<0)
    {
        ERROR_MSG("连接ctp行情服务器失败,rc=[%d]", rc);
        return -4;
    }
    //等待登录成功
    bool b_succ=false;
    for(int i=0; i<9; ++i)
    {
        if(CTP_SS_RUNNING==pCtpMd->get_conn_status())
        {
            b_succ = true;
            break;
        }
        else if(CTP_SS_INIT==pCtpMd->get_conn_status())//肯定登录失败了
        {
            b_succ = false;
            break;
        }
        
        os_thread_msleep(100);//休眠100毫秒   
    }
    
    if(!b_succ)
    {
        ERROR_MSG("登录ctp行情服务器失败当前登录状态[%c]", pCtpMd->get_conn_status());
        return -4;
    }
    
    //获取合约从文件获取或从ctp查询
    
    
    //订阅行情
    
    
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
