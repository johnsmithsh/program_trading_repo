
#include <string.h>

#include "msg_assemble.h"

//构建报文
int lmasm_connect(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char *ip, int port, char *errmsg)
{
    //ST_MSGLINK_BUFF linkmsg_ptr;

	//构造连接请求报文
	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	//设置报文头
	msglink_head_init(head_ptr);
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_CONN, MASKHEAD_REQ);//连接应答报文

	MSG_REQ_CONN *body_ptr=(MSG_REQ_CONN*)(&linkmsg_ptr->data_buff);
	memset(body_ptr, 0, sizeof(MSG_REQ_CONN));
	strncpy(body_ptr->group_no,   svr_link->link_info.group_no,      sizeof(body_ptr->group_no)-1);//!< 设置业务组号
	strncpy(body_ptr->group_desc, svr_link->link_info.group_desc,    sizeof(body_ptr->group_desc)-1);//!< 设置业务组描述信息
	//strncpy(body_ptr->version,    svr_link->link_info.group_version, sizeof(body_ptr->version));//!< 设置版本信息
	body_ptr->proc_id = (unsigned long)os_getpid();
	//body_ptr->prog_name,
	body_ptr->mode = 0;

	return 0;
}

int lmasm_ans_connect(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, unsigned int bcc_id, unsigned int bu_no, char if_succ, const char *szmsg, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_init(head_ptr);
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_CONN, MASKHEAD_RSP);//连接应答报文
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ANS_CONN);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	//设置链接应答信息
	MSG_ANS_CONN *body_ptr=(MSG_ANS_CONN*)linkmsg_ptr->data_buff;
	memset(body_ptr, 0, sizeof(MSG_ANS_CONN));
	body_ptr->bcc_id  = svr_link->link_info.bcc_id;
	body_ptr->bu_no   = svr_link->link_info.bu_no;
	body_ptr->if_succ = if_succ;//成功标记
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

   return -1;
}

//@brief 发送ack消息
int lmasm_ack(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle,  ST_MSG_HEAD *msg_head_ptr, char if_succ, char *szMsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, msg_head_ptr->msgid, MASKHEAD_ACK);//ack报文
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ACK);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	MSG_ACK *body_ptr=(MSG_ACK*)linkmsg_ptr->data_buff;
	body_ptr->if_succ = if_succ;//成功标记
	strncpy(body_ptr->szmsg, szMsg, sizeof(body_ptr->szmsg)-1);

	return 0;
}

int lmasm_register_function(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_REG_FUNC, MASKHEAD_REQ);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	int rc=msglink_pkg_data_append((unsigned char *)&linkmsg_ptr, sizeof(linkmsg_ptr), (unsigned char *)register_info,  len, errmsg);
	if(rc<0)
		return rc;
	head_ptr->data_len += len;

	return 0;
}

int lmasm_ans_register_function(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_REG_FUNC, MASKHEAD_RSP);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ANS_REGFUNC);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);


	MSG_ANS_REGFUNC *body_ptr = (MSG_ANS_REGFUNC *)linkmsg_ptr->data_buff;
	body_ptr->if_succ = if_succ;
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	return 0;
}

int lmasm_disconnect(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_DISCONN, MASKHEAD_REQ);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_REQ_DISCONN);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	MSG_REQ_DISCONN *body_ptr = (MSG_REQ_DISCONN*)linkmsg_ptr->data_buff;
	body_ptr->bcc_id = svr_link->link_info.bcc_id;
	body_ptr->bu_no  = svr_link->link_info.bu_no;
	strncpy(body_ptr->group_no, svr_link->link_info.group_no, sizeof(body_ptr->group_no)-1);

	return 0;
}

int lmasm_ans_disconnection(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_REG_FUNC, MASKHEAD_RSP);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_ANS_DISCONN);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);


	MSG_ANS_DISCONN *body_ptr = (MSG_ANS_DISCONN *)linkmsg_ptr->data_buff;
	body_ptr->bcc_id = svr_link->link_info.bcc_id;
	body_ptr->bu_no  = svr_link->link_info.bu_no;

	body_ptr->if_succ = if_succ;
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	return 0;
}


int lmasm_heatbeat(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_HEARTBEAT, MASKHEAD_REQ);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);

	return 0;
}

int lmasm_ans_heatbeat(ST_MSGLINK_BUFF * linkmsg_ptr, SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg)
{
	//ST_MSGLINK_BUFF linkmsg_ptr;

	ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;

	//设置报文头
	ST_MSG_HEAD *head_ptr= &linkmsg_ptr->head;
	msglink_head_set_msgtypeinfo(head_ptr, MSGTYPE_HEARTBEAT, MASKHEAD_RSP);//消息类型+请求/应答/ack标记
	head_ptr->data_len = sizeof(ST_MSG_COMMON)+sizeof(MSG_RSP);

	//设置common信息
	ST_MSG_COMMON *msg_common_ptr = &linkmsg_ptr->commoninfo;
	memset(msg_common_ptr, 0, sizeof(ST_MSG_COMMON));
	msglink_common_set_conninfo(msg_common_ptr, svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no);
	msglink_common_set_ctrlinfo(msg_common_ptr, true, false, false, false);


	MSG_RSP *body_ptr = (MSG_RSP *)linkmsg_ptr->data_buff;
	body_ptr->if_succ = if_succ;
	strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);

	return 0;
}
