#ifndef _APP_GLOBAL_INFO_H_
#define _APP_GLOBAL_INFO_H_
/*
 * @file:
 * @bref 该文件定义全局管理类型,用于整个进程内部资源统一管理
 * */
class CGlobalInfo
{
public:
	static CGlobalInfo*create_instance();
	static CGlobalInfo* get_instance();
	static bool destroy_instance();
public:
	CGlobalInfo();
	virtual ~CGlobalInfo();
public:
	CCtpMdConnection *get_ctp_md() { return m_pUserMdConn; }
public:
	CCtpMdConnection *m_pUserMdConn;//行情连接
};

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
#endif