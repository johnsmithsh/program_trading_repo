
#include "tx_session_handle.h"


CTxSessionQue::CTxSessionQue()
{
}

CTxSessionQue::~CTxSessionQue()
{
    //析构函数不能抛出异常
    try
    {
        std::deque<CTxSession *>::iterator iter;
        for(iter=m_session_que.begin(); iter!=m_session_que.end();)
        {
            if(NULL!=*iter)
            {
                delete (*iter);
                *iter=NULL;
            }
            iter = m_session_que.erase(iter);
        }
    }
    catch(...)
    {
    }
}

void CTxSessionQue::clear()
{
    AutoMutex(&m_mutex);
    std::deque<CTxSession *>::iterator iter;
    for(iter=m_session_que.begin(); iter!=m_session_que.end();)
    {
        if(NULL!=*iter)
        {
          delete (*iter);
          *iter=NULL;
        }
        iter = m_session_que.erase(iter);
    }
}

//@brief 分配request_id,并添加到队列中
CTxSession *CTxSessionQue::alloc_session()
{
    //todo 创建CTxSession
    CTxSession *pTxSession = new CTxSession();
    if(NULL==pTxSession)
        return NULL;
    
    //pTxSession->set_request_id(request_id);
    
    //todo 根据参数加入对应的队列
    {
        AutoMutex(&m_mutex);
        m_session_que.push_back(pTxSession);
    }

    return pTxSession;
}

//@brief 分配request_id,并添加到队列中
CTxSession *CTxSessionQue::alloc_session(int request_id)
{
    //todo 创建CTxSession
    if(request_id<=0)
        return NULL;
    
    CTxSession *pTxSession = alloc_session();
    if(NULL==pTxSession)
        return NULL;
    
    pTxSession->set_request_id(request_id);
    
    return pTxSession;
}


//@brief 根据指针查找session
CTxSession *CTxSessionQue::find_session(CTxSession *tx_session)
{
    AutoMutex(&m_mutex);
    CTxSession *ptr=NULL;
    std::deque<CTxSession *>::iterator iter;
    for(iter=m_session_que.begin(); iter!=m_session_que.end();++iter)
    {
        if(NULL==*iter)
            continue;
        
        ptr=*iter;
        if(tx_session==ptr)
            return *iter;
    }
    
    return NULL;
}

//@brief 根据request_id查找session
CTxSession *CTxSessionQue::find_session(int request_id)
{
    AutoMutex(&m_mutex);
    CTxSession *ptr=NULL;
    std::deque<CTxSession *>::iterator iter;
    for(iter=m_session_que.begin(); iter!=m_session_que.end();++iter)
    {
        if(NULL==*iter)
            continue;
        
        ptr=*iter;
        if(request_id==ptr->get_request_id())
            return *iter;
    }
    
    return NULL;
}

//@brief 按指针从队列中释放
void CTxSessionQue::free_session(CTxSession *tx_session)
{
    AutoMutex(&m_mutex);
    std::deque<CTxSession *>::iterator iter;
    for(iter=m_session_que.begin(); iter!=m_session_que.end();++iter)
    {
        if(NULL==*iter)
           continue;
        
        if(tx_session==*iter)
        {
          delete (*iter);
          *iter=NULL;
          m_session_que.erase(iter);
          break;
        }
    }
}
    
//@brief 按request_id从队列中释放
void CTxSessionQue::free_session(int request_id)
{
    AutoMutex(&m_mutex);
    CTxSession *ptr=NULL;
    std::deque<CTxSession *>::iterator iter;
    for(iter=m_session_que.begin(); iter!=m_session_que.end();++iter)
    {
        if(NULL==*iter)
            continue;
        
        ptr=*iter;
        if(request_id==ptr->get_request_id())
        {
            delete (*iter);
            *iter=NULL;
            m_session_que.erase(iter);
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

//@brief 创建交易会话句柄
TxSessnHandle tx_session_create(char session_type)
{
    //获取队列
    CTxSessionQue *pSessionQue=globalinfo_get_txSession(session_type);
    if(NULL==pSessionQue)
        return NULL;
    
    //创建会话
    CTxSession *pTxSession=pSessionQue->alloc_session();
    if(NULL==pTxSession)
        return NULL;
    
    return pTxSession;
    
    ////todo 创建CTxSession
    //CTxSession *pTxSession = new CTxSession();
    //if(NULL==pTxSession)
    //    return NULL;
    ////todo 根据参数加入对应的队列
    //CMarketDataDistributor *pCtpMdDistributor=globalinfo_get_mdDistributor();
    ////todo 是否获取request_id??
    //return NULL;
}

//@brief 根据request_id查询对象
TxSessnHandle tx_session_find_handle(char session_type, int request_id)
{
    //获取队列
    CTxSessionQue *pSessionQue=globalinfo_get_txSession(session_type);
    if(NULL==pSessionQue)
        return NULL;
    
    return pSessionQue->find_session(request_id);
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
int tx_session_delete(char session_type, TxSessnHandle &handle)
{
    if(NULL==handle)
        return 0;
    
    //获取队列
    CTxSessionQue *pSessionQue=globalinfo_get_txSession(session_type);
    if(NULL==pSessionQue)
        return -1;
    
    //释放对象
    pSessionQue->free_session(handle);
    handle = NULL;
    
    return 0;
    
    ////todo 根据状态从对应队列中移除
    ////todo 删除句柄
    //CTxSession *pTxSession = (CTxSession*)handle;
    //delete pTxSession;
    //handle = NULL;
    //return 0;
}


