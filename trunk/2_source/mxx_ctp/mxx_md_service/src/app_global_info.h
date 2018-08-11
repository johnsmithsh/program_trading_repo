#ifndef _APP_GLOBAL_INFO_H_
#define _APP_GLOBAL_INFO_H_
/*
 * @file:
 * @bref 该文件定义全局管理类型,用于整个进程内部资源统一管理
 * */
 
#include "ctp_md.h"
#include "thread_market_data.h"

class CGlobalInfo
{
public:
	//CGlobalInfo();
	virtual ~CGlobalInfo();
private:
    CGlobalInfo();
    CGlobalInfo(CGlobalInfo &obj);
    CGlobalInfo &operator=(CGlobalInfo &obj);
public:
    int init(const char *cfgfile);
public:
	CCtpMdConnection *get_ctp_md() { return m_pUserMdConn; }
public: //行情相关连接
	CCtpMdConnection       *m_pUserMdConn;//行情连接
    CMarketDataDistributor m_md_distributor;//行情分发器(线程)
    map<std::string, ST_MARKET_DATA> m_md_snapshot;//行情快照
    
public: //tx_session,用于存储请求应答数据的,
    CTxSessionQue m_qry_session_que;      //查询请求队列
    CTxSessionQue m_trade_session_que;    //交易队列,为了提交处理速度,交易队列单独列处理
    CTxSessionQue m_broadcast_session_que;//推送队列,如行情,交易所状态等
    CTxSessionQue m_multicast_session_que;//组播队列,如委托、成交回报等,只能发给对应的客户或席位
    
    //ItcMutex m_qry_mutex;
    //std::deque<CTxSession *> m_qry_que;//查询请求队列
    //ItcMutex m_trade_mutex;
    //std::deque<CTxSession *> m_trade_que;//交易队列,为了提交处理速度,交易队列单独列处理
    //ItcMutex m_broadcast_mutex;
    //std::deque<CTxSession *> m_broadcast_que;//推送队列,如行情,交易所状态等
    //ItcMutex m_multicast_mutex;
    //std::deque<CTxSession *> m_multicast_que;//组播队列,如委托、成交回报等,只能发给对应的客户或席位
    
public: //单实例模式 该实例在进程中只能有一个
	static CGlobalInfo* create_instance();
	static CGlobalInfo* get_instance();
	static bool         destroy_instance();
private:
    //由于该实例在程序启动就创建了,不会存在多线程并发创建
    //故该单实例模式不考虑这个问题
    static CGlobalInfo* m_instance;
    
};

//功能:获取行情连接
CCtpMdConnection *globalinfo_get_MdConnection();

//功能: 获取行情分发器
CMarketDataDistributor *globalinfo_get_mdDistributor();

//@brief 获取会话队列
CTxSessionQue *globalinfo_get_txSession(char session_type);
#endif
