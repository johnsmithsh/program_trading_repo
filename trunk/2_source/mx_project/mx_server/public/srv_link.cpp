#include "svr_link.h"

int msg_send(const char *buff, size_t data_len)
{
    return -1;
}

int msg_recv(const char *buff, size_t buffsize, int *data_len)
{
    reutrn -1;
}

int svrlink_create()
{
    return -1;
}

int svrlink_setinfo(SVRLINK_HANDLE svrlinkhandle, const char *group_no, const char *group_desc, const char *group_version, int pid)
{
    return 0;
}

int svrlink_connect(SVRLINK_HANDLE svrlinkhandle,  char *ip, int port, char *errmsg)
{
    ST_MSGLINK_BUFF msg_buff;
	memset(msg_buff, 0, sizeof(msg_buff.head+msg_Buff.commoninfo));
	
	int so;
	//todo 连接tcp服务器...
	
	//构造连接请求报文
	ST_SRV_LINK_HANDLE *svr_link=(ST_SRV_LINK_HANDLE*)svrlinkhandle;
	int rc=lmasm_req_conn(svr_link->link_info.group_no, svr_link->link_info.group_desc, svr_link->version, 0, &msg_buff, sizeof(msg_buff));
	if(rc<0)//构造报文错误
	{
	    return -1;
	}
	
	//发送连接请求
	rc=msg_send(&msg_buff, msg_buff.head.data_len+sizeof(msg_buff.head));
	if(rc<0)//!< 发送数据错误
	{
	    return -2;
	}
	
	//接收连接应答
	int data_len=0;
	rc=msg_recv(&msg_buff, sizeof(msg_buff), &data_len);
	if(rc<0)//接收连接回报消息错误
	{
	    return -3;
	}
	//rc=msglink_checkhead(msg_buff->head);//!< 校验报文头,接收时已经校验
	//if(rc<0)
	//{
	//    return -4;
	//}
	if(msg_buff.head.data_len<sizeof(msg_buff.head.commoninfo)+sizeof(MSG_ANS_CONN))//消息长度不正确
	    return -5;
	MSG_ANS_CONN *ans_info_ptr = (MSG_ANS_CONN*)msg_buff->data_buff;
	if(MSGLINK_YES!=ans_info_ptr->if_succ)//连接失败
	{
	    return -6;
	}
	
	svr_link->so=so;//!< tcp 连接描述符
	svr_link->link_info.bu_no= ans_info_ptr->bu_no;
	svr_link->link_info.bcc_id=ans_info_ptr->bcc_id;
	//todo 设置连接开始时间...
	
	return 0;
}

int svrlink_ans_connect(SVRLINK_HANDLE svrlinkhandle,  char if_succ, const char *szmsg, char *errmsg)
{
    return -1;
}

int svrlink_register_function(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg)
{
    ST_MSGLINK_BUFF msg_buff;
	memset(msg_buff, 0, sizeof(msg_buff.head+msg_Buff.commoninfo));
	
    MSG_REQ_REGFUNC;
	
	int rc=lmasm_req_regfunc_init(&msg_buff, sizeof(msg_buff));
	if(rc<0)//统计请求包失败
	{
	    return -2;
	}
	rc=msglink_data_append(register_info, len, msg_buff, sizeof(msg_buff));
	if(rc<0)
	{
	   return -3;
	}
    return 0;
}