#ifndef _MXX_TX_SESSION_HANDLE_H_
#define _MXX_TX_SESSION_HANDLE_H_
/*
 * 为了简化CTxSession的使用,增加该接口
 * 一次会话包括请求+应答;
 */

#include "tx_session.h"

typedef CTxSession* TxSessnHandle;

class CTxSessionQue
{
  public:
    CTxSessionQue();
    virtual ~CTxSessionQue();
  private:
    CTxSessionQue(CTxSessionQue &obj);
    CTxSessionQue &operator=(CTxSessionQue &obj);
  public:
    //@brief 清理数据
    void clear();
    
    //@brief 创建并添加到队列中
    CTxSession *alloc_session();
    //@brief 根据指针查找session
    CTxSession *find_session(CTxSession *tx_session);
    //@brief 按指针从队列中释放
    void    free_session(CTxSession *tx_session);
    
    //CTxSession *get_session(int request_id);
    
    
    //@brief 分配request_id,并添加到队列中
    CTxSession *alloc_session(int request_id);
    //@brief 根据request_id查找session
    CTxSession *find_session(int request_id);
    //@brief 按request_id从队列中释放
    void free_session(int request_id);
  public:
    ItcMutex m_mutex;
  private:
    std::deque<CTxSession *> m_session_que;//
};

////////////////////////////////////////////////////////////////////////////
//会话类型   每个类型代表一个队列
#define TXSESSION_TYPE_QUERY     '0' //类型: 查询
#define TXSESSION_TYPE_TRADE     '1' //类型: 交易
#define TXSESSION_TYPE_BROADCAST '2' //类型: 广播
#define TXSESSION_TYPE_MULTICAST '3' //类型: 组播

//@brief 创建交易会话句柄
TxSessnHandle tx_session_create(char session_type);
//@brief 根据request_id查询对象
TxSessnHandle tx_session_find_handle(char session_type, int request_id);
//@brief 删除对象
int           tx_session_delete(char session_type, TxSessnHandle &handle);

//@brief 交易会话句柄设置requestID与api接口中的requestID必须一致,否则spi的OnRspXxx找不到对应的请求
//注:不是线程安全
int tx_session_set_request_id(TxSessnHandle handle, int request_id);

//int tx_session_register_function(TxSessnHandle handle, OnRspSuccess, OnRspFail);

//@brief 交易会话句柄请求数据;
//注: 如果OnRsp数据不需要,则可以不设置该函数;
//注:不是线程安全
int tx_session_set_tx_in(TxSessnHandle handle, void *data_ptr, size_t data_len);

//@brief 等待交易会话结束; OnRsp会设置会话状态
//注:不是线程安全
int tx_session_wait(TxSessnHandle handle, int ms_to);

//@brief 获取对象下一个数据
//注:不是线程安全
void tx_session_next_tx_out(TxSessnHandle handle, void *data_ptr, size_t data_len);



#endif
