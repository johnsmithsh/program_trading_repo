#ifndef __MXX_CTP_MD_H_
#define __MXX_CTP_MD_H_

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
	virtual void OnHeartBeatWarning(int nTimeLapse);
	

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
#define CTP_SS_CONNECTING '1'  //已连接
#define CTP_SS_LOGINNING  '2'  //登录中
#define CTP_SS_RUNNING    '3'  //正在运行
#define CTP_SS_LOGOUTING  '4'  //登出中

class CCtpConnection
{
  public:
    CCtpConnection();
    virtual ~CCtpConnection();
  public:
    int init(const char *flow_path);
    int release();
    int connect_sync(const char *front_addr, const char *ns_addr);
    int login_sync(const char *user_name, const char *pwd);
    int logout_sync();
    int SubscribeMarketData_sync();
    int UnSubscribeMarketData_sync();
    int SubscribeForQuoteRsp_sync();
    int UnSubscribeForQuoteRsp_sync();
  public:
    char m_ctp_status;//ctp连接状态
    CThostFtdcMdApi *m_md_api;//请求接口
    CCtpMdSpi *m_md_spi; //应答处理接口
};
#endif
