
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