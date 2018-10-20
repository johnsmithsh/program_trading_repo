#ifndef __MXX_MSG_ASSEMBLE_H_
#define __MXX_MSG_ASSEMBLE_H_
/*
 * @file 构建常用报文
 *
 **/
#include "msg_link_define.h"
#include "svr_link.h" //提供SVRLINK_HANDLE

//组建报文 link msg assemble: lmasm
int lmasm_ack(ST_MSGLINK_BUFF  *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle,  ST_MSG_HEAD *msg_head_ptr, char if_succ);
//@连接请求
int lmasm_connect(ST_MSGLINK_BUFF     *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle);
int lmasm_ans_connect(ST_MSGLINK_BUFF *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, unsigned int bcc_id, unsigned int bu_no, char if_succ, const char *szmsg);

//@注册业务功能
int lmasm_register_function(ST_MSGLINK_BUFF     * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg);
int lmasm_ans_register_function(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg);

//@断开链接
int lmasm_disconnect(ST_MSGLINK_BUFF        *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle);
int lmasm_ans_disconnection(ST_MSGLINK_BUFF *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg);

//@心跳报文
int lmasm_heatbeat(ST_MSGLINK_BUFF     *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle);
int lmasm_ans_heatbeat(ST_MSGLINK_BUFF *linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg);

//构建通用报文
int lmasm_begin(ST_MSGLINK_BUFF  *linkmsg_ptr, unsigned int msgtype);
//@追加数据
int lmasm_append(ST_MSGLINK_BUFF  *linkmsg_ptr, unsigned char *data_ptr, size_t len);
//@是否还有后续数据
int lmasm_end(bool next_flag=false);

#endif
