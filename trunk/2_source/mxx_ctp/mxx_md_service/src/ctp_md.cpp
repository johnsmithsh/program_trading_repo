#include "ctp_md.h"
#include "app_global_info.h"

///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void CCtpMdSpi::OnFrontConnected()
{
	CGlobalInfo *pGlobalInfo=CGlobalInfo::get_instance();
	if(NULL==pGlobalInfo)
		return;
	CCtpMdConnection *pCtpMd=pGlobalInfo->get_ctp_md();
	if(NULL==pCtpMd)
		return;
	INFO_MSG("行情连接成功!");

	pCtpMd->set_conn_status(CTP_SS_CONNECTED);//已连接

	int rc;
	if(pCtpMd->get_autoconn_flag())//自动连接
	{
		ERROR_MSG("发起请求请求...");
		rc=pCtpMd->login();
		if(rc<0)
		{
			ERROR_MSG("登录失败[%d]!", rc);
		}
	}

	return;
}

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
void CCtpMdSpi::OnFrontDisconnected(int nReason)
{
	INFO_MSG("OnFrontDisconnected行情连接断开[0x%x]...", nReason);
	CCtpMdConnection *pCtpMd=globalinfo_get_MdConnection();
	if(NULL==pCtpMd)
		return;

	char conn_status = pCtpMd->get_conn_status();
	pCtpMd->set_conn_status(CTP_SS_INIT);
	INFO_MSG("行情连接状态: [%c]=>[%c]", conn_status, CTP_SS_INIT);

	int rc;
	if(pCtpMd->get_autoconn_flag())//自动连接
	{
		INFO_MSG("重新发起行情连接请求...", conn_status, CTP_SS_INIT);
		rc=pCtpMd->connect();
		if(rc<0)
		{
			ERROR_MSG("重新发起行情连接请求失败[%d]!", rc);
		}
	}

	return;
}

///登录请求响应
void CCtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	INFO_MSG("OnRspUserLogin行情登录应答:requestId[%d], [%d] [%s]...", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	INFO_MSG("OnRspUserLogin:交易日期[%s],brokerID[%s],user[%s],systemName[%s]", pRspUserLogin->TradingDay, pRspUserLogin->BrokerID, pRspUserLogin->UserID,pRspUserLogin->SystemName);
	INFO_MSG("loginTime=[%s], sessionID[%d], frontID=[%d], maxOrderRef=[%d]",pRspUserLogin->LoginTime,pRspUserLogin->SessionID,pRspUserLogin->FrontID, pRspUserLogin->MaxOrderRef);
	INFO_MSG("shfeTime=[%s], dceTime=[%s], czceTime=[%s], cffexTime=[%s], ineTime=[%s]",pRspUserLogin->SHFETime,pRspUserLogin->DCETime,pRspUserLogin->CZCETime, pRspUserLogin->FFEXTime,pRspUserLogin->INETime);

	bool b_succ=false;//登录成功标记
	if(0==pRspInfo->ErrorID) b_succ=true;

	CCtpMdConnection *pCtpMd=globalinfo_get_MdConnection();
	if(NULL==pCtpMd)
		return;
	char conn_status = pCtpMd->get_conn_status();
	if(!b_succ)//登录失败
	{
		pCtpMd->set_conn_status(CTP_SS_CONNECTED);
		INFO_MSG("行情连接状态: [%c]=>[%c]", conn_status, CTP_SS_CONNECTED);
		return;
	}

	pCtpMd->set_conn_status(CTP_SS_RUNNING);
	INFO_MSG("行情连接状态: [%c]=>[%c]", conn_status, CTP_SS_RUNNING);

	//todo 记录登录时间,登录会话, 前置id
	//todo 记录各个交易所时间
	//todo 通知客户端各个交易所时间

	return;
}

///登出请求响应
void CCtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	INFO_MSG("OnRspUserLogout行情登录应答:requestId[%d], [%d] [%s]...", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	INFO_MSG("OnRspUserLogout:brokerID[%s],user[%s],systemName[%s]", pUserLogout->BrokerID, pUserLogout->UserID);

	bool b_succ=false;//登录成功标记
	if(0==pRspInfo->ErrorID) b_succ=true;

	CCtpMdConnection *pCtpMd=globalinfo_get_MdConnection();
	if(NULL==pCtpMd)
		return;
	char conn_status = pCtpMd->get_conn_status();
	if(!b_succ)//登录失败
	{
		pCtpMd->set_conn_status(CTP_SS_RUNNING);
		INFO_MSG("行情连接状态: [%c]=>[%c]", conn_status, CTP_SS_RUNNING);
		return;
	}

	pCtpMd->set_conn_status(CTP_SS_INIT);
	INFO_MSG("行情连接状态: [%c]=>[%c]", conn_status, CTP_SS_INIT);

	//todo 通知客户端登出

	return;
}
///错误应答
void CCtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	INFO_MSG("OnRspError:requestId[%d], [%d] [%s]...", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	//INFO_MSG("OnRspError:brokerID[%s],user[%s],systemName[%s]", pUserLogout->BrokerID, pUserLogout->UserID);

	//todo 通知客户端错误消息

	return;
}
///订阅行情应答
void CCtpMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}
///取消订阅行情应答
void CCtpMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}
///订阅询价应答
void CCtpMdSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}
///取消订阅询价应答
void CCtpMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}
///深度行情通知
void CCtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{

}
///询价通知
void CCtpMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
}
///////////////////////////////////////////////////////////////////////////////////
CCtpMdConnection::CCtpMdConnection()
{
  m_ctp_status=CTP_SS_INIT;
  m_md_api=NULL;
  m_md_spi=NULL;
}

CCtpMdConnection::~CCtpMdConnection()
{
  set_autoconn_flag(false);//防止端口后自动重连;
  if(NULL==m_md_api)
  {
	  m_pMdApi->Release();
	  m_pMdApi=NULL;
  }
  
  if(NULL==m_pMdSpi)
  {
    delete m_pMdSpi;
    m_pMdSpi=NULL;
  }
}

int CCtpMdConnection::init(const char *flow_path /*="./ctp_md_path/"*/)
{
	if(NULL!=m_pMdApi)
		return 0;
	INFO_MSG("初始化行情API...");
	m_pMdApi=CThostFtdcMdApi::CreateFtdcMdApi(flow_path, false, false);
	if(NULL=m_pMdApi)
	{
		ERROR_MSG("Error: 创建行情API实例失败");
		return -1;
	}
	m_pMdSpi=new CCtpMdSpi();
	if(NULL==m_pMdSpi)
	{
		ERROR_MSG("Error: 创建行情SPI实例失败");
		return -2;
	}
	m_pMdApi->RegisterSpi(m_pMdSpi);

	INFO_MSG("mdApi libversion[%s]", m_pMdApi->GetApiVersion());
	return 0;
}

int CCtpMdConnection::release()
{
	set_autoconn_flag(false);//防止端口后自动重连;
	if(NULL==m_md_api)
	{
		m_pMdApi->Release();
		//没有delete??
		m_pMdApi=NULL;
	}

	if(NULL==m_pMdSpi)
	{
	  delete m_pMdSpi;
	  m_pMdSpi=NULL;
	}

	return 0;
}

void CCtpMdConnection::set_tx_date(const char *tx_date)
{
	if(NULL!=tx_date)
		strncpy(m_tx_date, tx_date, sizeof(m_tx_date)-1);
}

int CCtpMdConnection::connect_sync(const char *front_addr, const char *ns_addr)
{
	INFO_MSG("连接行情服务器:前置[%s],域名服务器[%s]...", front_addr, ns_addr);
	if((NULL!=front_addr)&&('\0'!=*front_adddr))
	{
		m_pMdApi->RegisterFront((char*)front_addr);
	}
	else if((NULL!=ns_addr)&&('\0'!=*ns_addr))
	{
		m_pMdApi->RegisterNameServer((char*)ns_addr);
	}
	else
	{
		ERROR_MSG("Error: 前置[%s],域名服务器[%s]不能全部为空", front_addr, ns_addr);
		return -1;
	}

	m_pMdApi->Init();
	m_pMdApi->Join();

	return 0;
}

int CCtpMdConnection::login_sync(const char *broker_id, const char *user_name, const char *user_pwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));

	set_autoconn_flag(true);//断开后自动重连;

	INFO_MSG("登录行情服务器:经纪商[%s],用户[%s]...", broker_id, user_name);
	strcpy(req.BrokerID, broker_id);
	strcpy(req.UserID,   user_name);
	strcpy(req.Password, user_pwd);
	int rc=m_pMdApi->ReqUserLogin(&req, get_requestid());
	if(rc<0)
	{
		ERROR_MSG("Error: API登录行情失败!rc=[%d]", rc);
		return rc;
	}

	return 0;
}

int CCtpMdConnection::logout_sync()
{
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));

	set_autoconn_flag(false);//防止断开后自动重连;

	strcpy(req.BrokerID, m_borker_id);
	strcpy(req.UserID,   m_user_name);
	int rc=m_pMdApi->ReqUserLogout(&req, get_requestid());
	if(rc<0)
	{
		ERROR_MSG("Error: API登出行情失败!rc=[%d]", rc);
		return rc;
	}

	return 0;
}

int CCtpMdConnection::SubscribeMarketData_sync(std::vector<std::string> contract_vec)
{
	if(contract_vec.empty())
		return 0;
	int count=contract_vec.size();
	char (*ppInstrumentID)[32];//每个合约长度为32

	ppInstrumentID=malloc(count*32);
	if(NULL==ppInstrumentID)
		return -1;

	//todo contract_vec转换为ppInstrumentID;

	int rc=m_pMdApi->SubscribeMarketData(ppInstrumentID, count);
	if(rc<0)
	{
		ERROR_MSG("Error: API订阅行情失败!rc=[%d]", rc);
		return rc;
	}

	return rc;
}

int CCtpMdConnection::UnSubscribeMarketData_sync(std::vector<std::string> contract_vec)
{
	if(contract_vec.empty())
		return 0;
	int count=contract_vec.size();
	char (*ppInstrumentID)[32];//每个合约长度为32

	ppInstrumentID=malloc(count*32);
	if(NULL==ppInstrumentID)
		return -1;

	//todo contract_vec转换为ppInstrumentID;

	int rc=m_pMdApi->UnSubscribeMarketData(ppInstrumentID, count);
	if(rc<0)
	{
		ERROR_MSG("Error: API取消订阅行情失败!rc=[%d]", rc);
		return rc;
	}

	return rc;
}


int CCtpMdConnection::SubscribeForQuoteRsp_sync(std::vector<std::string> contract_vec)
{
	if(contract_vec.empty())
		return 0;
	int count=contract_vec.size();
	char (*ppInstrumentID)[32];//每个合约长度为32

	ppInstrumentID=malloc(count*32);
	if(NULL==ppInstrumentID)
		return -1;

	//todo contract_vec转换为ppInstrumentID;

	int rc=m_pMdApi->SubscribeForQuoteRsp(ppInstrumentID, count);
	if(rc<0)
	{
		ERROR_MSG("Error: API订阅询价行情失败!rc=[%d]", rc);
		return rc;
	}

	return rc;
}

int CCtpMdConnection::UnSubscribeForQuoteRsp_sync(std::vector<std::string> contract_vec)
{
	if(contract_vec.empty())
		return 0;
	int count=contract_vec.size();
	char (*ppInstrumentID)[32];//每个合约长度为32

	ppInstrumentID=malloc(count*32);
	if(NULL==ppInstrumentID)
		return -1;

	//todo contract_vec转换为ppInstrumentID;

	int rc=m_pMdApi->UnSubscribeForQuoteRsp(ppInstrumentID, count);
	if(rc<0)
	{
		ERROR_MSG("Error: API取消订阅询价行情失败!rc=[%d]", rc);
		return rc;
	}

	return rc;
}
