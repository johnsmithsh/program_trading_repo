
#include <stdlib.h>
#include <string.h>

#include "tx_session.h"

CTxSession::CTxSession() : m_tx_in(NULL),m_request_id(0),m_tx_status(TX_SS_INIT),m_retCode(0)
{
    m_szMsg[0]='\0';
}

CTxSession::~CTxSession()
{
    if(NULL!=m_tx_in)
    {
        free(m_tx_in);
        m_tx_in = NULL;
    }
    
    std::deque<ST_TX_DATA *>::iterator iter;
    for(iter=m_tx_out.begin(); iter!=m_tx_out.end(); )
    {
        if(NULL!=*iter)
        {
            free(*iter);
            *iter=NULL;
        }
        
        iter = m_tx_out.erase(iter);
    }
    
}


//@brief 增加输入数据
int CTxSession::add_tx_in(void *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0))
        return 0;
    if(NULL!=m_tx_in)
        return -1;
    
    unsigned char *ptr=(unsigned char *)malloc(sizeof(ST_TX_DATA)+data_len);
    if(NULL==ptr)
        return -2;
    
    ST_TX_DATA *pTxData = (ST_TX_DATA*)ptr;
    pTxData->data_ptr = (unsigned char *)ptr + sizeof(ST_TX_DATA);
    pTxData->size = data_len;
    memcpy(pTxData->data_ptr, data_ptr, data_len);
    pTxData->data_len=data_len;
    
    m_tx_in = pTxData;
    return 0;
}

//@brief 增加交易输出数据
int CTxSession::add_tx_out(void *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0))
        return 0;
    
    unsigned char *ptr=(unsigned char *)malloc(sizeof(ST_TX_DATA)+data_len);
    if(NULL==ptr)
        return -2;
    
    ST_TX_DATA *pTxData = (ST_TX_DATA*)ptr;
    pTxData->data_ptr = (unsigned char *)ptr + sizeof(ST_TX_DATA);
    pTxData->size = data_len;
    memcpy(pTxData->data_ptr, data_ptr, data_len);
    pTxData->data_len=data_len;
    
    m_tx_out.push_back(pTxData);
    
    return 0;
}


//@brief 设置错误码和错误信息
void CTxSession::set_code_msg(int retCode, char *szMsg)
{
    m_retCode = retCode;
    memset(m_szMsg, 0, sizeof(m_szMsg));
        
    if((NULL!=szMsg)&&('\0'!=*szMsg))
    {
        strncpy(m_szMsg, szMsg, sizeof(m_szMsg)-1);
    }
}