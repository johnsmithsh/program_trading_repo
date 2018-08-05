
#include <errno.h>
#include <stdio.h>

#include "logfile.h"
#include "ConfigFile.h"
#include "thread_market_data.h"

//构造函数
CMarketDataDistributor::CMarketDataDistributor(const char *thread_name/*="recv_thread"*/)
    :Thread_Base(thread_name),m_b_running(false),m_stop_flag(false)
{
   m_b_running=false;
   m_stop_flag=false;
}

//析构函数
CMarketDataDistributor::~CMarketDataDistributor()
{
    m_stop_flag=true;
}

int CMarketDataDistributor::init()
{
   return 0;
}

int CMarketDataDistributor::terminate_service()
{
    m_stop_flag=true;
	
    return 0;
}

void CMarketDataDistributor::run()
{
    m_b_running=true;
	this->service_routine();
	m_b_running=false;
}

int CMarketDataDistributor::loadini(char *cfgfile)
{
    //ConfigFile cfg;
    //int rc=cfg.load_cfg_file(cfgfile);
    //if(rc < 0)
    //{
    //    ERROR_MSG("打开配置文件[%s]失败, rc=[%d]", cfgfile, rc);
    //    return -1;
    //}
    //
    //char serve_section[]="sourth_server";
    //m_lstn_port   =cfg.read_int(serve_section,  "serverport", 9999);//!< 服务监听端口
    //m_max_listen  =cfg.read_int(serve_section,  "max_conn",   1024); //!< 最大连接数
    //m_recv_timeout=cfg.read_int(serve_section,  "timeout",    1000); //!< 接收超时,单位毫秒
    //m_send_timeout = m_recv_timeout; //!< 发送超时时间,单位毫秒
    //
    //if( ((m_recv_timeout<0)||(m_recv_timeout>3000))
    //    ||((m_send_timeout<0)||(m_send_timeout>3000))
    //  )
    //{
    //    FATAL_MSG("[%s][%s]超时时间配置不合法!范围[0,3000]", serve_section, "timeout");
    //    return -1;
    //}
    
    return 0;   
}

void CMarketDataDistributor::process_market_data(ST_MARKET_DATA *md)
{
    if(NULL==md)
        return;
    
    AutoMutex autlock(&m_mutex);
    m_md_que.push_back(*md);
}

int CMarketDataDistributor::service_routine() 
{
  int rc;  
  
  INFO_MSG("CMarketDataDistributor: start thread...");

  int errcode;
  
  while((!m_stop_flag) //线程停止指令
	   )
  {
    m_b_running=true;
    
    push_msg_to_client();
  }//继续等待

  //清除连接
  m_b_running=false;

  return 0;
}


//行情消息推送到客户端
//即:放入对应的客户端发送线程的缓存即可;
int CMarketDataDistributor::push_msg_to_client()
{
    if(m_md_que.empty())
        return -1;
    char szMsg[2018];
    
    {
        std::deque<ST_MARKET_DATA>::iterator iter;
        
        //此处逻辑有3种实现
        //1. 逐个socket推送;
        //2. 每个socket一个推送线程;
        //3. 一个线程处理一组socket的推送;
        AutoMutex autolock(&m_mutex);
        for(iter=m_md_que.begin(); iter!=m_md_que.end(); ++iter)
        {
            ST_MARKET_DATA &md_ref=*iter;
            memset(szMsg, 0, sizeof(szMsg));
            sprintf(szMsg, 
                       "time=%s.%d"
                       ",tradingday=%s"
                       ",exch_code=%s"
                       ",contract_code=%s"
                       ",LastPrice=%f"
                       ",PreSettlementPrice=%f"
                       ",PreClosePrice=%f"
                       ",PreOpenInterest=%f"
                       ",OpenPrice=%f"
                       ",HighestPrice=%f"
                       ",LowestPrice=%f"
                       ",Volume=%d"
                       ",Turnover=%f"
                       ",OpenInterest=%f"
                       ",ClosePrice=%f"
                       ",SettlementPrice=%f"
                       ",UpperLimitPrice=%f"
                       ",LowerLimitPrice=%f"
                       ",PreDelta=%f"
                       ",CurrDelta=%f"
                       ",BidPrice1=%f"
                       ",BidVolume1=%d"
                       ",AskPrice1=%f"
                       ",AskVolume1=%d"
                       ",BidPrice2=%f"
                       ",BidVolume2=%d"
                       ",AskPrice2=%f"
                       ",AskVolume2=%d"
                       ",BidPrice3=%f"
                       ",BidVolume3=%d"
                       ",AskPrice3=%f"
                       ",AskVolume3=%d"
                       ",BidPrice4=%f"
                       ",BidVolume4=%d"
                       ",AskPrice4=%f"
                       ",AskVolume4=%d"
                       ",BidPrice5=%f"
                       ",BidVolume5=%d"
                       ",AskPrice5=%f"
                       ",AskVolume5=%d"
                       ",AveragePrice=%f"
                       ",ActionDay=%s"
                       ,
                       md_ref.UpdateTime,md_ref.UpdateMillisec,
                       md_ref.TradingDay,
                       md_ref.ExchangeID,
                       md_ref.InstrumentID,
                       md_ref.LastPrice,
                       md_ref.PreSettlementPrice,
                       md_ref.PreClosePrice,
                       md_ref.PreOpenInterest,
                       md_ref.OpenPrice,
                       md_ref.HighestPrice,
                       md_ref.LowestPrice,
                       md_ref.Volume,
                       md_ref.Turnover,
                       md_ref.OpenInterest,
                       md_ref.ClosePrice,
                       md_ref.SettlementPrice,
                       md_ref.UpperLimitPrice,
                       md_ref.LowerLimitPrice,
                       md_ref.PreDelta,
                       md_ref.CurrDelta,
                       md_ref.BidPrice1,
                       md_ref.BidVolume1,
                       md_ref.AskPrice1,
                       md_ref.AskVolume1,
                       md_ref.BidPrice2,
                       md_ref.BidVolume2,
                       md_ref.AskPrice2,
                       md_ref.AskVolume2,
                       md_ref.BidPrice3,
                       md_ref.BidVolume3,
                       md_ref.AskPrice3,
                       md_ref.AskVolume3,
                       md_ref.BidPrice4,
                       md_ref.BidVolume4,
                       md_ref.AskPrice4,
                       md_ref.AskVolume4,
                       md_ref.BidPrice5,
                       md_ref.BidVolume5,
                       md_ref.AskPrice5,
                       md_ref.AskVolume5,
                       md_ref.AveragePrice,
                       md_ref.ActionDay
                   );
            
            INFO_MSG("%s", szMsg);
        }
        
        m_md_que.erase(m_md_que.begin(), m_md_que.end());
    }
    
    return 0;
}
