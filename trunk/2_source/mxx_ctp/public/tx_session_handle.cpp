
#include "tx_session_handle.h"

typedef CTxSession* TxSessnHandle;

//@brief 创建交易会话句柄
TxSessnHandle tx_session_create()
{
    //todo 创建CTxSession
    CTxSession *pTxSession = new CTxSession();
    if(NULL==pTxSession)
        return NULL;
    //todo 根据参数加入对应的队列??
    //todo 是否获取request_id??
    return NULL;
}

//@brief 交易会话句柄设置requestID与api接口中的requestID必须一致,否则spi的OnRspXxx找不到对应的请求
int tx_session_set_request_id(TxSessnHandle handle, int request_id)
{
    if((NULL==handle)||(request_id<=0))
        return -1;
    CTxSession *pTxSession =(CTxSession*)handle;
    pTxSession->set_request_id(request_id);
    
    return 0;
}

//@brief 交易会话句柄请求数据;
//注: 如果OnRsp数据不需要,则可以不设置该函数;
int tx_session_set_tx_in(TxSessnHandle handle, void *data_ptr, size_t data_len)
{
    if((NULL==handle)
        return -1;
    if((NULL==data_ptr)||(data_len<=0))
        return -2;
    
    CTxSession *pTxSession =(CTxSession*)handle;
    pTxSession->add_tx_in(data_ptr, data_len);
    
    return 0;
}

//@brief 等待交易会话结束; OnRsp会设置会话状态
int tx_session_wait(TxSessnHandle handle, int ms_to)
{
    if(NULL==handle)
        return -1;
    CTxSession *pTxSession = (CTxSession*)handle;
    for(;;)
    {
        if( (TX_SS_FINISH==pTxSession->get_tx_status())  //会话完结
            ||(TX_SS_ERROR==pTxSession->get_tx_status()) //会话错误
          )
          break;
          
        //todo 判断是否超时...
    }
}

//@brief 等待交易会话结束; OnRsp会设置会话状态
void tx_session_next_tx_out(TxSessnHandle handle, void *data_ptr, size_t data_len)
{
}

//@brief 等待交易会话结束; OnRsp会设置会话状态
int tx_session_delete(TxSessnHandle &handle)
{
    if(NULL==handle)
        return 0;
    //todo 根据状态从对应队列中移除
    //todo 删除句柄
    CTxSession *pTxSession = (CTxSession*)handle;
    delete pTxSession;
    handle = NULL;
    return 0;
}


