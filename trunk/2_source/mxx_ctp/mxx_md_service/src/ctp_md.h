#ifndef __MXX_CTP_MD_H_
#define __MXX_CTP_MD_H_

#include <string>
#include <vector>

#include "ThostFtdcMdApi.h"

class CCtpMdSpi: public CThostFtdcMdSpi
{
  public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason);
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	///@desc 该接口是内部实现,不会被调用
	virtual void OnHeartBeatWarning(int nTimeLapse) {}
	

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///订阅询价应答
	virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///取消订阅询价应答
	virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) ;

	///询价通知
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) ;
};

//ctp连接状态
#define CTP_SS_INIT       '0'  //初始状态
#define CTP_SS_CONNECTING '1'  //连接中
#define CTP_SS_CONNECTED  '2'  //已连接
#define CTP_SS_LOGINNING  '3'  //登录中
#define CTP_SS_RUNNING    '4'  //正在运行
#define CTP_SS_LOGOUTING  '5'  //登出中

class CCtpMdConnection
{
  public:
    CCtpMdConnection();
    virtual ~CCtpMdConnection();
  public:
    int load_ini(const char *cfgfile);
  public:
    int init();
    int release();

    //功能: 连接
    int connect() { return connect_sync(m_front_addr, m_ns_addr); }
    //功能: 登录
    int login()   { return login_sync(m_borker_id, m_user_name, m_user_pwd); }
    //功能: 登出
    int logout()  { return logout_sync();}

    //功能: 订阅行情;
    int SubscribeMarketData_sync(std::vector<std::string> &contract_vec);
    //功能: 退订行情
    int UnSubscribeMarketData_sync(std::vector<std::string> &contract_vec);

    int SubscribeForQuoteRsp_sync(std::vector<std::string> &contract_vec);
    int UnSubscribeForQuoteRsp_sync(std::vector<std::string> &contract_vec);

  public:
    //功能:分配请求序号
    int get_requestid(){ return ++m_requestid;}

    //功能: 设置自动重连标记
    void set_autoconn_flag(bool flag=true) { m_bAutoReconn=flag; }
    //功能:获取自动重连标记
    bool get_autoconn_flag(){ return m_bAutoReconn;}

    //功能:设置连接状态
    char get_conn_status() { return m_ctp_status;}

    void set_conn_status(char conn_status) { m_ctp_status=conn_status; }

    //功能:设置交易日期
    void set_tx_date(const char *tx_date);
    const char *get_tx_date() { return m_tx_date; }
  public:
    //自动重连
    bool m_bAutoReconn;//true-断开后自动重连;
    char            m_ctp_status;//ctp连接状态
    CThostFtdcMdApi *m_pMdApi;//API请求接口
    CCtpMdSpi       *m_pMdSpi;      //SPI应答处理接口
    
  private:
    int connect_sync(const char *front_addr, const char *ns_addr);
    int login_sync(const char *broker_id, const char *user_name, const char *pwd);
    int logout_sync();
  private:
    int  m_requestid;//请求序号

    char m_flow_path[256];//!< ctp数据流文件保存路径
    char m_front_addr[64];//前置地址
    char m_ns_addr[64];   //域名服务器地址

    char m_borker_id[64];//期货公司会员号
    char m_user_name[64];//用户名,即user_id
    char m_user_pwd[128];//用户密码
    char m_tx_date[16];//交易日期
  private:
    int  m_session_id;
    int  m_front_id;
    char m_login_time[12];
};
#endif
