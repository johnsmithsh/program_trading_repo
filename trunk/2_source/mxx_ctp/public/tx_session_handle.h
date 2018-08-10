#ifndef _MXX_TX_SESSION_HANDLE_H_
#define _MXX_TX_SESSION_HANDLE_H_
/*
 * 为了简化CTxSession的使用,增加该接口
 * 一次会话包括请求+应答;
 */

#include "tx_session.h"

typedef CTxSession* TxSessnHandle;

//@brief 创建交易会话句柄
TxSessnHandle tx_session_create();

//@brief 交易会话句柄设置requestID与api接口中的requestID必须一致,否则spi的OnRspXxx找不到对应的请求
int tx_session_set_request_id(TxSessnHandle handle, int request_id);

//@brief 交易会话句柄请求数据;
//注: 如果OnRsp数据不需要,则可以不设置该函数;
int tx_session_set_tx_in(TxSessnHandle handle, void *data_ptr, size_t data_len);

//@brief 等待交易会话结束; OnRsp会设置会话状态
int tx_session_wait(TxSessnHandle handle, int ms_to);

//@brief 等待交易会话结束; OnRsp会设置会话状态
void tx_session_next_tx_out(TxSessnHandle handle, void *data_ptr, size_t data_len);

//@brief 等待交易会话结束; OnRsp会设置会话状态
int tx_session_delete(TxSessnHandle &handle);

#endif
