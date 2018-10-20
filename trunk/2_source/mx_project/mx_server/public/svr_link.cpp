#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "svr_link.h"
//#include "msg_link_define.h"
#include "msg_link_function.h"

#include "os_thread.h"

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

int            svrhandle_get_socket(SVRLINK_HANDLE   svrlinkhandle)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    return svr_link->so;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <sys/socket.h>
/**
 * @brief 发送数据; 要么发送一个完整数据包,要么返回错误; 发送半个数据包也返回错误
 *        阻塞方式发送数据;
 * @param
 *   [in]so: socket文件描述符
 *   [in]buff,data_len: 发送数据及数据长度
 *   [out]send_len: 最终发送数据长度
 *   [out]szmsg: 错误消息
 * @retval
 *    >=0-成功, 且表示发送字节数
 *   MSG_SOCK_CLOSE-发送过程中对方关闭socket;
 *    <0-其他错误
 */
int msglink_send(int so, const unsigned char *buff, size_t data_len, int *send_len, char *szmsg)
{
   *send_len = 0;
   if(NULL!=szmsg) *szmsg='\0';
   if(data_len<=0) return 0;

   int rc=0;
   int count = 0;//发送计数
   int retry_count = 0;//重试次数
	for(;count<data_len;)
	{
		rc = send(so, buff+count, data_len-count, 0);
		if(0==rc)
		{
			if(NULL!=szmsg)
				sprintf(szmsg, "msglink_send failed! socket peer close");
			return MSG_SOCK_CLOSE;//socket关闭
		}
		else if(rc < 0)
		{
			int err_code=errno;
		    // 当send收到信号时,可以继续写
		   if(err_code == EINTR)//
		        continue;
		    // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
		    // 在这里做延时后再重试.
		   else if(err_code == EAGAIN)
		    {
		       ++retry_count;
		       if(retry_count>3)//重试超过次数,则退出
		         {
		           break;
		         }

		       usleep(10);
		       continue;
		    }
		   if(NULL!=szmsg)
			   sprintf(szmsg, "msglink_send failed! errno=[%d], strerr=[%s]", err_code, strerror(err_code));
		   return err_code;
		}

		count += rc;
	}

	return count;
}

//@brief 接收数据; 要么接收一个完整数据包,要么返回错误
int msglink_recv(int so, unsigned char *buff, size_t buffsize, int *recv_len, char *szmsg)
{
	*recv_len = 0;
	int err_code=0;
	if(NULL!=szmsg) *szmsg='\0';

	ST_MSG_HEAD msg_head;
	memset(&msg_head, 0, sizeof(ST_MSG_HEAD));
	//peek报文头
	int rc=0;
	for(;;)
	{
		rc=recv(so, &msg_head, sizeof(ST_MSG_HEAD), MSG_PEEK|MSG_DONTWAIT);
		if(0==rc)//socket在读取过程中连接关闭
		{
			return MSG_SOCK_CLOSE;
		}
		else if(rc<0)
		{
			err_code = errno;
			if((EAGAIN==err_code) || (EWOULDBLOCK==err_code))//没有数据
			{
				return MSG_SOCK_NO_DATA;
			}
			else if(EINTR==err_code)//被中断,继续读取吧
			{
				continue;//继续读取
			}
			else //其他错误
			{
				if(NULL!=szmsg)
				{
					sprintf(szmsg, "peek报文头失败!未知错误,errno=[%d], strerr=[%s]", err_code, strerror(err_code));
				}
				return err_code;
			}
		}
	}//enf of for

	//校验报文头
	int data_size = msg_head.data_len + sizeof(msg_head);//数据总长度
	int count = 0; //已接收数据长度
	if(data_size>(int)buffsize)//正常来讲,不应该出现该错误,如果出现该错误,说明出现问题了,如这不是个报文头 or 上一个数据头指定100字节,实际发送小于100字节
	{
		if(NULL!=szmsg) strcpy(szmsg, "缓存不足");
		return MSG_BUFF_ENOMEM;
	}

	while(count<data_size)
	{
	    /*
	     * 默认socket的recv是阻塞的; 无论阻塞与非阻塞,rc<0表示失败; =0连接关闭; >0-收到数据;
	     *     当数据没有到达,默认情况下(阻塞模式)recv会等待数据到达;
	     * 注意: rc<0 &&((errno==EINTR)||(errno==EWOULDBLOCK)||(errno==EAGAIN)) 认为是正常的,继续接收数据
	     *   当非阻塞模式下,如果socket没有可用数据,则返回-1,并将errno设置为EAGAIN或EWOULDBLOCK
	     **/
	   rc=recv(so, buff+count, data_size-count,0);
	   if(0==rc)//连接关闭
	    {
		   return MSG_SOCK_CLOSE;
	    }
	   else if(rc<0)
	    {
		   err_code = errno;
	        //非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
	      if( (err_code==EAGAIN) || (EWOULDBLOCK==err_code) ) //socket被标记为非阻塞,接收被阻塞或超时,没有数据
	         break;
	      else if(EINTR==err_code)//在收到数据前,被信号中断
	         continue;
	      if(NULL!=szmsg)
	     		sprintf(szmsg, "报文接收错误,errno=[%d], strerr=[%s]", err_code, strerror(err_code));
	      return err_code;
	    }

	   count +=rc;
	   *recv_len = count;
	}

	return count;
}

/**
 * @brief peek报文头,即读取报文头后,报文头数据仍然存在于socket缓存区,仍然需要接收;
 *      当确切知道要接收数据长度的情况下,才能调用该函数; 如已经知道报文有多长,
 * @param
 *   [in]so: socket
 *   [out]msg_head_ptr: 报文头缓冲区
 *   [out]szmsg: 错误消息
 * @retval 0-peek成功;
 *   MSG_SOCK_CLOSE/MSG_SOCK_HALFPACK; <0-其他错误;
 * @note
 *     此处仅仅peek指定长度数据,并未校验报文头数据有效性
 */
int msglink_peekhead(int so, ST_MSG_HEAD *msg_head_ptr, char *szmsg)
{
	memset(msg_head_ptr, 0, sizeof(ST_MSG_HEAD));
	if(NULL!=szmsg) *szmsg='\0';

	int rc       = 0;
	int err_code = 0;

	//这个for循环主要是为了在EINTR错误时重试;
	for(;;)
	{
		rc=recv(so, msg_head_ptr, sizeof(ST_MSG_HEAD), MSG_PEEK|MSG_DONTWAIT);
		if(0==rc)//socket在读取过程中连接关闭
		{
			return MSG_SOCK_CLOSE;
		}
		else if(rc<0)
		{
			err_code = errno;
			if((EAGAIN==err_code) || (EWOULDBLOCK==err_code))//没有数据
			{
				return MSG_SOCK_NO_DATA;
			}
			else if(EINTR==err_code)//被中断,继续读取吧
			{
				continue;//继续读取
			}
			else //其他错误
			{
				if(NULL!=szmsg)
				{
					sprintf(szmsg, "peek报文头失败!未知错误,errno=[%d], strerr=[%s]", err_code, strerror(err_code));
				}
				return err_code;
			}
		}

		if(rc!=sizeof(ST_MSG_HEAD))
			return MSG_SOCK_HALFPACK;
		return 0;
	}

	return 0;
}

/**
 * @brief (阻塞)接收数据;
 *      当确切知道要接收数据长度的情况下,才能调用该函数; 如已经知道报文有多长,
 * @param
 *   [in]so: socket
 *   [out]buff: 接收缓冲区
 *   [in]datalen: 接收数据长度; 调用者必须保证缓存长足充足
 *   [out]recv_len: 接收数据长度
 *   [out]szmsg: 错误消息
 * @retval >=0-返回接收字节数;
 *   MSG_SOCK_CLOSE; <0-其他错误;
 * @note
 *     需要校验接收数据长度与数据报文长度是否匹配
 */
int msglink_recvdata(int so, unsigned char *buff, size_t datalen, int *recv_len, char *szmsg)
{
	if((NULL==buff)||(datalen<=0)) return 0;

	int data_size = (int)datalen;
	int count = 0;
	int rc=0;
	int err_code=0;//这是为了防止errno多线程竞争出现问题定义的变量; 实际上gcc已经处理该问题,即每个线程有一个errno单独的变量定义

	while(count<data_size)
	{
		    /*
		     * 默认socket的recv是阻塞的; 无论阻塞与非阻塞,rc<0表示失败; =0连接关闭; >0-收到数据;
		     *     当数据没有到达,默认情况下(阻塞模式)recv会等待数据到达;
		     * 注意: rc<0 &&((errno==EINTR)||(errno==EWOULDBLOCK)||(errno==EAGAIN)) 认为是正常的,继续接收数据
		     *   当非阻塞模式下,如果socket没有可用数据,则返回-1,并将errno设置为EAGAIN或EWOULDBLOCK
		     **/
		   rc=recv(so, buff+count, data_size-count,0);
		   if(0==rc)//连接关闭
		    {
			   return MSG_SOCK_CLOSE;
		    }
		   else if(rc<0)
		    {
			   err_code = errno;
		        //非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
		      if( (err_code==EAGAIN) || (EWOULDBLOCK==err_code) ) //socket被标记为非阻塞,接收被阻塞或超时,没有数据
		         break;
		      else if(EINTR==err_code)//在收到数据前,被信号中断
		         continue;
		      if(NULL!=szmsg)
		     		sprintf(szmsg, "报文接收错误,errno=[%d], strerr=[%s]", err_code, strerror(err_code));
		      return err_code;
		    }

		   count +=rc;
		   *recv_len = count;
	}

	return count;

}
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
