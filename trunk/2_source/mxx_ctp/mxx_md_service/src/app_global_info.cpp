
#include "app_global_info.h"

//功能:获取行情连接
CCtpMdConnection *globalinfo_get_MdConnection()
{
	CGlobalInfo *pGlobalInfo=CGlobalInfo::get_instance();
	if(NULL==pGlobalInfo)
		return NULL;
	CCtpMdConnection *pCtpMd=pGlobalInfo->get_ctp_md();
	if(NULL==pCtpMd)
		return NULL;
	return pCtpMd;
}

//功能: 获取行情分发器
CMarketDataDistributor *globalinfo_get_mdDistributor()
{
	CGlobalInfo *pGlobalInfo=CGlobalInfo::get_instance();
	if(NULL==pGlobalInfo)
		return NULL;
	return &(pGlobalInfo->m_md_distributor);
}

CTxSessionQue *globalinfo_get_txSession(char session_type)
{
    CGlobalInfo *pGlobalInfo=CGlobalInfo::get_instance();
	if(NULL==pGlobalInfo)
		return NULL;
    switch(session_type)
    {
        case TXSESSION_TYPE_QUERY:     //类型: 查询
            return &(pGlobalInfo->m_qry_session_que);
        case TXSESSION_TYPE_TRADE:     //类型: 交易
            return &(pGlobalInfo->m_trade_session_que);    //交易队列
        case TXSESSION_TYPE_BROADCAST: //类型: 广播
            return &(pGlobalInfo->m_broadcast_session_que);//推送队列
        case TXSESSION_TYPE_MULTICAST: //类型: 组播    
            return &(pGlobalInfo->m_multicast_session_que);//组播队列
        default:
            return NULL;
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////
CGlobalInfo *CGlobalInfo::m_instance=NULL;

CGlobalInfo *CGlobalInfo::create_instance()
{
    if(NULL==m_instance)
    {
        m_instance = new CGlobalInfo();
    }
    
    return m_instance;
}

CGlobalInfo *CGlobalInfo::get_instance()
{
    return m_instance;
}

bool CGlobalInfo::destroy_instance()
{
    if(NULL!=m_instance)
        delete m_instance;
    m_instance=NULL;
    return true;
}

///////////////////////////////////////////////////////////////////////
CGlobalInfo::CGlobalInfo():m_pUserMdConn(NULL)
{
}

CGlobalInfo::~CGlobalInfo()
{
    if(NULL!=m_pUserMdConn)
    {
        delete m_pUserMdConn;
    }
    
    m_pUserMdConn = NULL;
}

int CGlobalInfo::init(const char *cfgfile)
{
    if(NULL==m_pUserMdConn)
    {
        m_pUserMdConn = new CCtpMdConnection();
        if(NULL==m_pUserMdConn)
        {
            ERROR_MSG("ERROR: faild to new CCtpMdConnection!");
            return -1;
        }
        if(m_pUserMdConn->init(cfgfile)<0)
        {
            ERROR_MSG("ERROR: failed to init CCtpMdConnection!");
            return -2;
        }
    }
    
    
    return 0;
}