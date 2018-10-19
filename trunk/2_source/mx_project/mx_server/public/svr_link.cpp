#include <stdlib.h>
#include <string.h>

#include "svr_link.h"
//#include "msg_link_define.h"
#include "msg_link_function.h"

#include "os_thread.h"

int msg_send(const char *buff, size_t data_len)
{
    return -1;
}

int msg_recv(const char *buff, size_t buffsize, int *data_len)
{
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//连接句柄
SVRLINK_HANDLE svrhandle_open()
{
	ST_SVR_LINK_HANDLE *svr_link = (ST_SVR_LINK_HANDLE*)malloc(sizeof(ST_SVR_LINK_HANDLE));
	if(NULL==svr_link)
		return NULL;
	return svr_link;
}
void svrhandle_close(SVRLINK_HANDLE &svrlinkhandle)
{
	free(svrlinkhandle);
	svrlinkhandle = NULL;
}
int svrhandle_init(SVRLINK_HANDLE svrlinkhandle)
{
	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
	memset(svr_link, 0, sizeof(ST_SVR_LINK_HANDLE));
	return 0;
}
int svrhandle_set_groupinfo(SVRLINK_HANDLE svrlinkhandle, const char *group_no, const char *group_desc, const char *group_version, int pid)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

    strncpy(svr_link->link_info.group_no,   group_no,   sizeof(svr_link->link_info.group_no)-1);
    strncpy(svr_link->link_info.group_desc, group_desc, sizeof(svr_link->link_info.group_desc)-1);
    return 0;
}

int svrhandle_set_linkinfo(SVRLINK_HANDLE svrlinkhandle, unsigned int bcc_id, unsigned int bu_no)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    svr_link->link_info.bcc_id = bcc_id;
    svr_link->link_info.bu_no  = bu_no;

    return 0;
}

int svrhandle_set_socket(SVRLINK_HANDLE svrlinkhandle, int so)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    svr_link->so = so;
    return 0;
}

unsigned int svrhandle_next_serial(SVRLINK_HANDLE svrlinkhandle)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    return ++svr_link->send_serial;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int svrlink_connect(SVRLINK_HANDLE svrlinkhandle,  char *ip, int port, char *errmsg)
{
    ST_MSGLINK_BUFF msg_buff;
	
	int so;
	//todo 连接tcp服务器...
	
	//构造连接请求报文
	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	//设置报文头
	msglink_head_init(head_ptr);
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_CONN, MASKHEAD_REQ);//连接应答报文

	MSG_REQ_CONN *body_ptr=(MSG_REQ_CONN*)(&msg_buff.data_buff);
	memset(body_ptr, 0, sizeof(MSG_REQ_CONN));
	strncpy(body_ptr->group_no,   svr_link->link_info.group_no,      sizeof(body_ptr->group_no)-1);//!< 设置业务组号
	strncpy(body_ptr->group_desc, svr_link->link_info.group_desc,    sizeof(body_ptr->group_desc)-1);//!< 设置业务组描述信息
	//strncpy(body_ptr->version,    svr_link->link_info.group_version, sizeof(body_ptr->version));//!< 设置版本信息
	body_ptr->proc_id = (unsigned long)os_getpid();
	//body_ptr->prog_name,
	body_ptr->mode = 0;

	/*
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
	*/
	//todo 设置连接开始时间...
	
	return 0;
}

int svrlink_ans_connect(SVRLINK_HANDLE svrlinkhandle, unsigned int bcc_id, unsigned int bu_no, char if_succ, const char *szmsg, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_init(head_ptr);
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_CONN, MASKHEAD_RSP);//连接应答报文
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ANS_CONN);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	//设置链接应答信息
	MSG_ANS_CONN *body_ptr=(MSG_ANS_CONN*)msg_buff.data_buff;
	memset(body_ptr, 0, sizeof(MSG_ANS_CONN));
	body_ptr->bcc_id = svr_link->link_info.bcc_id;
	body_ptr->bu_no  = svr_link->link_info.bu_no;
	body_ptr->if_succ = if_succ;//成功标记
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	//todo send...
   return -1;
}

//@brief 发送ack消息
int svrlink_ack(SVRLINK_HANDLE svrlinkhandle, ST_MSG_HEAD *msg_head_ptr, char if_succ, char *szMsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, msg_head_ptr->msgid, MASKHEAD_ACK);//ack报文
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ACK);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	MSG_ACK *body_ptr=(MSG_ACK*)msg_buff.data_buff;
	body_ptr->if_succ = if_succ;//成功标记
	strncpy(body_ptr->szmsg, szMsg, sizeof(body_ptr->szmsg)-1);

	//todo send..
	return 0;
}

int svrlink_register_function(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_REG_FUNC, MASKHEAD_REQ);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);
	
	int rc=msglink_pkg_data_append((unsigned char *)&msg_buff, sizeof(msg_buff), (unsigned char *)register_info,  len, errmsg);
	if(rc<0)
		return rc;
	head_ptr->data_len += len;

	//int rc=lmasm_req_regfunc_init(&msg_buff, sizeof(msg_buff));
	//if(rc<0)//统计请求包失败
	//{
	//    return -2;
	//}
	//rc=msglink_data_append(register_info, len, msg_buff, sizeof(msg_buff));
	//if(rc<0)
	//{
	//   return -3;
	//}

	//todo send...

	return 0;
}

int svrlink_ans_register_function(SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_REG_FUNC, MASKHEAD_RSP);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ANS_REGFUNC);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);


	MSG_ANS_REGFUNC *body_ptr = (MSG_ANS_REGFUNC *)msg_buff.data_buff;
	body_ptr->if_succ = if_succ;
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	//todo send...
	return 0;
}


int svrlink_disconnect(SVRLINK_HANDLE svrlinkhandle, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_DISCONN, MASKHEAD_REQ);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_REQ_DISCONN);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	MSG_REQ_DISCONN *body_ptr = (MSG_REQ_DISCONN*)msg_buff.data_buff;
	body_ptr->bcc_id = svr_link->link_info.bcc_id;
	body_ptr->bu_no  = svr_link->link_info.bu_no;
	strncpy(body_ptr->group_no, svr_link->link_info.group_no, sizeof(body_ptr->group_no)-1);

	//todo send...

	return 0;
}

int svrlink_ans_disconnection(SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_REG_FUNC, MASKHEAD_RSP);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ANS_DISCONN);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);


	MSG_ANS_DISCONN *body_ptr = (MSG_ANS_DISCONN *)msg_buff.data_buff;
	body_ptr->bcc_id = svr_link->link_info.bcc_id;
	body_ptr->bu_no  = svr_link->link_info.bu_no;

	body_ptr->if_succ = if_succ;
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	//todo send...
	return 0;
}


int svrlink_heatbeat(SVRLINK_HANDLE svrlinkhandle, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_HEARTBEAT, MASKHEAD_REQ);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	//todo send...

	return 0;
}

int svrlink_ans_heatbeat(SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg)
{
	ST_MSGLINK_BUFF msg_buff;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &msg_buff.head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_HEARTBEAT, MASKHEAD_RSP);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_RSP);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &msg_buff.commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);


	MSG_RSP *body_ptr = (MSG_RSP *)msg_buff.data_buff;
	body_ptr->if_succ = if_succ;
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	//todo send...
	return 0;
}
