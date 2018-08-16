#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mxx_net_socket.h"
#include "msg_link_define.h"
#include "msg_link_function.h"
#include "svr_link.h"

int msglink_send(int so, const unsigned char *buff, size_t data_len, char *errmsg)
{
    int snd_len=0;
    int rc=0;

    rc=mxx_socket_send_waitall(so, (char *)buff, data_len, &snd_len, 0);
    if(rc<0)
    {
        if(MXX_SOCKET_TIMEOUT==rc)//超时
        {
            if(NULL!=errmsg) sprintf(errmsg, "发送msg错误:rc=[%d]发送超时!", rc);
        }
        else 
        {
            if(NULL!=errmsg) sprintf(errmsg, "发送msg错误:rc=[%d]发送错误[%s]!", rc, strerror(rc));
        }
        return rc;
    }
    
    if(snd_len!=int(data_len))
    {
        if(NULL!=errmsg) sprintf(errmsg, "发送msg错误:数据发送不完整[%d<%d]!", snd_len, (int)data_len);
        return -2;
    }
    
    return 0;
}

int msglink_recv(int so, unsigned char *buff, size_t buffsize, int *data_len, char *err_msg)
{
    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)) )
        return -1;
    int recv_len=0;
    int rc=0;
    
    //接收报文头
    rc=mxx_socket_recv_waitall(so, (char *)buff, sizeof(ST_MSG_HEAD), &recv_len, 0);
    if(rc<0)
    {
        if(NULL!=err_msg)
        {
            if(MXX_SOCKET_TIMEOUT==rc)
                sprintf(err_msg, "接收head错误:接收超时");
            else if(MXX_SOCKET_CLOSED==rc)
                sprintf(err_msg, "接收head错误:socket已关闭");
            else 
                sprintf(err_msg, "接收head错误:rc=[%d], strerr=[%s]", rc, strerror(rc));
        }
        return -2;
    }
    if(recv_len!=sizeof(ST_MSG_HEAD))
    {
        if(NULL!=err_msg) sprintf(err_msg, "接收报文头失败!");
        return -3;
    }
    
    if(NULL!=data_len) 
        *data_len=recv_len;
    
    //校验报文头...
    
    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
    if(head_ptr->data_len<=0)//不存在业务数据
    {
        
        return 0;
    }
    if(buffsize<head_ptr->data_len+sizeof(ST_MSG_HEAD))
    {
        if(NULL!=err_msg) 
            sprintf(err_msg, "接收缓存区不足,buffsize=[%d], msg link data=[%lu]", (int)buffsize, head_ptr->data_len+sizeof(ST_MSG_HEAD));
        return -4;
    }
        
    //接收报文体
    recv_len=0;
    rc=mxx_socket_recv_waitall(so, (char *)buff+sizeof(ST_MSG_HEAD), head_ptr->data_len, &recv_len, 1000);
    if(NULL!=data_len) 
        *data_len += recv_len;
    if(recv_len==int(head_ptr->data_len))
    {
        return 0;
    }
    else if(rc<0)
    {
        if(NULL!=err_msg)
        {
            if(MXX_SOCKET_TIMEOUT==rc)
                sprintf(err_msg, "接收body错误: 超时");
            else if(MXX_SOCKET_CLOSED==rc)
                sprintf(err_msg, "接收body错误: socket已关闭");
            else 
                sprintf(err_msg, "接收body错误: rc=[%d], strerr=[%s]", rc, strerror(rc));
        }
    }
    
    return 0;
}

/** @brief 接收消息msg数据包,一次只能接收一个
 *  @param
 *    [out]recv_buff: msg缓存;
 *    [out]errmsg: 错误消息
 *  @retval
 *     0-成功; <0-失败;
 **/
int svrlink_recv(SVRLINK_HANDLE svrlinkhandle,  ST_MSGLINK_BUFF *recv_buff, char *errmsg)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    memset(&recv_buff->head, 0, sizeof(recv_buff->head));
    return msglink_recv(svr_link->so, (unsigned char *)recv_buff, sizeof(ST_MSGLINK_BUFF), NULL, errmsg);
}

/** @brief 发送消息msg数据包 需要使用者填充报文头;
 *  @param
 *    [in]send_buff: 需要发送的msg消息
 *    [out]errmsg: 错误消息
 *  @retval
 *     0-成功; <0-失败;
 **/
int svrlink_send(SVRLINK_HANDLE svrlinkhandle,  ST_MSGLINK_BUFF *send_buff, char *errmsg)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    
    return msglink_send(svr_link->so, (unsigned char *)send_buff, send_buff->head.data_len+sizeof(ST_MSG_HEAD), errmsg);
}

//----------------------------------------------------------------------------------------------------------------------

bool svrlink_valid(SVRLINK_HANDLE svrlinkhandle)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    if(NULL==svr_link) return false;
    
    //socket无效,
    if(svr_link->so<=0) return false;
    
    return 0;
}

//@brief 创建连接句柄; 返回连接句柄; NULL-创建失败;
SVRLINK_HANDLE svrlink_create()
{
    unsigned char *ptr=(unsigned char *)malloc(sizeof(ST_SVR_LINK_HANDLE));
    if(NULL==ptr) return NULL;
    memset(ptr, 0, sizeof(ST_SVR_LINK_HANDLE));
    
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE *)ptr;
    svr_link->so=MXX_SOCKET_INVSOCK;//socket无效
    
    return (SVRLINK_HANDLE)ptr;
}

/** @brief 创建连接句柄; NULL-创建失败;
 *  @param
 *    [in]group_no: 组号; 业务进程存在组号; 控制中心没有组号;
 *    [in]group_desc: 业务进程的业务说明;
 *    [in]group_version: 组号版本信息说明
 *    [in]pid: 进程号
 * @retval
 *     连接句柄; NULL-创建失败;
 **/
SVRLINK_HANDLE svrlink_create(const char *group_no, const char *group_desc, const char *group_version, int pid)
{
    SVRLINK_HANDLE svr_link=svrlink_create();
    if(NULL==svr_link)
        return NULL;
    int rc=svrlink_setbuinfo(svr_link, group_no, group_desc, group_version, pid);
    if(rc<0)
    {
        free(svr_link);
        svr_link=NULL;
    }
    return svr_link;
}

/** @brief 设置业务进程连接信息
 *  @param
 *    [in]group_no: 组号; 业务进程存在组号; 控制中心没有组号;
 *    [in]group_desc: 业务进程的业务说明;
 *    [in]group_version: 组号版本信息说明
 *    [in]pid: 进程号
 * @retval
 *     0-成功; <0-失败;
 **/
int svrlink_setbuinfo(SVRLINK_HANDLE svrlinkhandle, const char *group_no, const char *group_desc, const char *group_version, int pid)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    if(NULL==svr_link)
        return -1;
    ST_LINK_INFO *link_info_ptr=&svr_link->link_info;
    strncpy(link_info_ptr->group_no,      group_no, sizeof(link_info_ptr->group_no)-1);
    strncpy(link_info_ptr->group_desc,    group_desc, sizeof(link_info_ptr->group_desc)-1);
    //strncpy(link_info_ptr->group_version, group_version, sizeof(link_info_ptr->group_version)-1);
    //link_info_ptr->pid=pid;
    return 0;
}

/** @brief 设置业务号(控制中心分配)
 *  @param
 *    [in]bu_no: bu_id; 控制中心分配
 * @retval
 *     0-成功; <0-失败;
 **/
int svrlink_setbuno(SVRLINK_HANDLE svrlinkhandle, int bu_no)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    if(NULL==svr_link)
        return -1;
    ST_LINK_INFO *link_info_ptr=&svr_link->link_info;
    link_info_ptr->bu_no=bu_no;
}

/** @brief 设置控制中心信息
 *  @param
 *    [in]bcc_id: 组号; 业务进程存在组号; 控制中心没有组号;
 *    [in]bcc_version: 业务进程的业务说明;
 * @retval
 *     0-成功; <0-失败;
 **/
int svrlink_setbccinfo(SVRLINK_HANDLE svrlinkhandle, int bcc_id)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    if(NULL==svr_link)
        return -1;
    ST_LINK_INFO *link_info_ptr=&svr_link->link_info;
    link_info_ptr->bcc_id=bcc_id;
    return 0;
}

//@brief 删除svrlink_create创建的句柄; 0-成功; <0-失败;
int svrlink_close(SVRLINK_HANDLE svrlinkhandle)
{
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    if(NULL==svr_link)
        return 0;
    
    if(svr_link->so>0)
    {
        mxx_socket_delete(svr_link->so);
        svr_link->so=MXX_SOCKET_INVSOCK;
    }
    
    free(svr_link);
    return 0;
}


/** 
 * @brief 客户端=>控制中心: 连接请求
 *     MSGTYPE_CONN
 *   连接控制中心前必须使用svrlink_setxxx配置当前业务服务说明信息;
 * @param
 *   [in]ip:      控制中心ip地址;
 *   [in]port:    控制中心端口号;
 *   [out]errmsg: 错误消息
 * @retval
 *   0-成功; <0-失败;
 **/
int svrlink_connect(SVRLINK_HANDLE svrlinkhandle,  char *ip, int port, char *errmsg)
{
    ST_MSGLINK_BUFF msg_buff;
    memset(&msg_buff, 0, sizeof(msg_buff.head)+sizeof(msg_buff.commoninfo));
    
    //构造连接请求报文
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    int rc=lmasm_req_conn(svr_link->link_info.group_no, svr_link->link_info.group_desc, svr_link->version, 0, (char *)&msg_buff, sizeof(msg_buff));
    if(rc<0)//构造报文错误
    {
        return -1;
    }
    
    //连接控制中心...
    int so;
    so=mxx_socket_create();
    rc=mxx_socket_connect(so, ip, port, 0);
    if(rc<0)
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "连接控制中心[%s:%d]失败,rc=[%d]", ip, port, rc);
        mxx_socket_delete(so);//!< 关闭socket
        return rc;
    }
    
    //发送连接请求...
    rc=msglink_send(so, (unsigned char *)&msg_buff, msg_buff.head.data_len+sizeof(msg_buff.head), errmsg);
    if(rc<0)//!< 发送数据错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu=>cc, 发送连接请求包失败[%s:%d]失败,rc=[%d]", ip, port, rc);
        mxx_socket_delete(so);//!< 关闭socket
        return -2;
    }
    
    //接收连接应答
    int data_len=0;
    rc=msglink_recv(so, (unsigned char *)&msg_buff, sizeof(msg_buff), &data_len, errmsg);
    if(rc<0)//接收连接回报消息错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu<=cc,接收连接请求应答包失败[%s:%d]失败,rc=[%d]", ip, port, rc);
        mxx_socket_delete(so);//!< 关闭socket
        return -3;
    }
    //rc=msglink_checkhead(msg_buff->head);//!< 校验报文头,接收时已经校验
    //if(rc<0)
    //{
    //    return -4;
    //}
    if(msg_buff.head.data_len<sizeof(msg_buff.commoninfo)+sizeof(MSG_ANS_CONN))//消息长度不正确
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "连接请求应答包数据错误[%s:%d]失败", ip, port);
        mxx_socket_delete(so);//!< 关闭socket
        return -5;
    }
    MSG_ANS_CONN *ans_info_ptr = (MSG_ANS_CONN*)msg_buff.data_buff;
    if(C_YES!=ans_info_ptr->if_succ)//连接失败
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "控制中心[%s:%d]拒绝连接", ip, port);
        mxx_socket_delete(so);//!< 关闭socket
        return -6;
    }
    
    svr_link->so=so;//!< tcp 连接描述符
    svr_link->link_info.bu_no= ans_info_ptr->bu_no; //!< 控制中心分配的业务号码
    svr_link->link_info.bcc_id=ans_info_ptr->bcc_id; //!< 控制中心id
    //todo 设置连接开始时间...
    
    return 0;
}

int svrlink_ans_connect(SVRLINK_HANDLE svrlinkhandle,  char if_succ, const char *szmsg, char *errmsg)
{
    int rc;
    
    ST_MSGLINK_BUFF msg_buff;
    memset(&msg_buff, 0, sizeof(msg_buff.head)+sizeof(msg_buff.commoninfo));
    
    //构造报文
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    rc=lmasm_ans_conn(svr_link->link_info.bu_no, svr_link->link_info.bcc_id, if_succ, szmsg, (char *)&msg_buff, sizeof(msg_buff));
    if(rc<0)//构造报文错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu<=cc, 构建连接应答报文失败,rc=[%d]", rc);
        return -1;
    }
    
    //发送连接应答...
    rc=msglink_send(svr_link->so, (unsigned char *)&msg_buff, msg_buff.head.data_len+sizeof(msg_buff.head), errmsg);
    if(rc<0)//!< 发送数据错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu<=cc, 发送连接应答报文失败,rc=[%d]", rc);
        return -2;
    }
    
    //服务器,不会自动关闭,让客户端关闭吧!...
    return 0;
}

int svrlink_begin_register(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg)
{
    return -1;
}
int svrlink_register_function(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg)
{
    int rc;
    ST_MSGLINK_BUFF msg_buff;
    memset(&msg_buff, 0, sizeof(msg_buff.head)+sizeof(msg_buff.commoninfo));
    
    rc=lmasm_req_regfunc_init((char *)&msg_buff, sizeof(msg_buff));
    if(rc<0)//统计请求包失败
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu=>cc, 构建业务功能注册数据失败,rc=[%d]", rc);
        return -2;
    }
    rc=msglink_pkg_data_append((unsigned char *)&msg_buff, sizeof(msg_buff), (unsigned char *)register_info, len, errmsg);
    if(rc<0)
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu=>cc, 构建业务功能注册数据失败,rc=[%d]", rc);
        return -3;
    }
    
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    //发送...
    rc=msglink_send(svr_link->so, (unsigned char *)&msg_buff, msg_buff.head.data_len+sizeof(msg_buff.head), errmsg);
    if(rc<0)//!< 发送数据错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu=>cc, 发送业务功能注册失败,rc=[%d]", rc);
        return -2;
    }
    
    //接收应答...
    int data_len=0;
    rc=msglink_recv(svr_link->so, (unsigned char *)&msg_buff, sizeof(msg_buff), &data_len, errmsg);
    if(rc<0)//接收连接回报消息错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu<=cc,接收连接请求应答包失败失败,rc=[%d]", rc);
        mxx_socket_delete(svr_link->so);//!< 关闭socket
        return -3;
    }
    //rc=msglink_checkhead(msg_buff->head);//!< 校验报文头,接收时已经校验
    //if(rc<0)
    //{
    //    return -4;
    //}
    if(msg_buff.head.data_len<sizeof(msg_buff.commoninfo)+sizeof(MSG_ANS_CONN))//消息长度不正确
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "连接请求应答包数据错误失败");
        mxx_socket_delete(svr_link->so);//!< 关闭socket
        return -5;
    }
    MSG_ANS_REGFUNC *ans_info_ptr = (MSG_ANS_REGFUNC*)msg_buff.data_buff;
    if(C_YES!=ans_info_ptr->if_succ)//连接失败
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "控制中心拒绝连接");
        mxx_socket_delete(svr_link->so);//!< 关闭socket
        return -6;
    }
    return 0;
}
int svrlink_end_register(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg)
{
    return -1;
}

//@brief 发出请求数据 MSGTYPE_REQ_REQUEST
int svrlink_request(SVRLINK_HANDLE svrlinkhandle, unsigned char *data_ptr, size_t data_len, char *errmsg)
{
    int rc;
    
    //校验数据...
    if((NULL==data_ptr)||(data_len<=0))
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "数据是空数据");
        return -1;
    }
    
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    //校验连接是否有效...
    if(!svrlink_valid(svrlinkhandle))
    {
        if(NULL!=errmsg)
            sprintf(errmsg, "msglink无效");
        return -1;
    }
    
    
    ST_MSGLINK_BUFF msg_buff;
    memset(&msg_buff, 0, sizeof(msg_buff.head)+sizeof(msg_buff.commoninfo));
    
    rc=msglink_pkg_head_init((unsigned char *)&msg_buff, sizeof(ST_MSGLINK_BUFF), MSGTYPE_REQ_REQUEST, errmsg);
    rc=msglink_pkg_conninfo((unsigned char *)&msg_buff, sizeof(ST_MSGLINK_BUFF), svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no, errmsg);
    
    //设置控制信息...
    
    //添加业务数据
    rc=msglink_pkg_data_append((unsigned char *)&msg_buff, sizeof(ST_MSGLINK_BUFF), data_ptr, data_len, errmsg);
    if(rc<0)
        return rc;
    
    //发送数据
    rc=msglink_send(svr_link->so, (unsigned char *)&msg_buff, msg_buff.head.data_len+sizeof(msg_buff.head), errmsg);
    if(rc<0)//!< 发送数据错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu=>cc, 发送业务功能注册失败,rc=[%d]", rc);
        return -2;
    }
    
    return 0;
}

//@brief 发送数据,建议每次传输多个完整的业务报文;
//    MSGTYPE_DATA
int svrlink_transfer(SVRLINK_HANDLE svrlinkhandle, unsigned char *data_ptr, int data_len, bool first_flag, bool next_flag, char *errmsg)
{
    int rc;
    
    //校验数据...
    if((NULL==data_ptr)||(data_len<=0))
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "数据是空数据");
        return -1;
    }
    
    ST_SVR_LINK_HANDLE *svr_link=(ST_SVR_LINK_HANDLE*)svrlinkhandle;
    //校验连接是否有效...
    if(!svrlink_valid(svrlinkhandle))
    {
        if(NULL!=errmsg)
            sprintf(errmsg, "msglink无效");
        return -1;
    }

    ST_MSGLINK_BUFF msg_buff;
    memset(&msg_buff, 0, sizeof(msg_buff.head)+sizeof(msg_buff.commoninfo));
    
    rc=msglink_pkg_head_init((unsigned char *)&msg_buff, sizeof(ST_MSGLINK_BUFF), MSGTYPE_DATA, errmsg);
    rc=msglink_pkg_conninfo((unsigned char *)&msg_buff, sizeof(ST_MSGLINK_BUFF), svr_link->link_info.bcc_id, svr_link->link_info.bu_no, svr_link->link_info.group_no, errmsg);

    //设置控制信息...
    
    //添加业务数据
    rc=msglink_pkg_data_append((unsigned char *)&msg_buff, sizeof(ST_MSGLINK_BUFF), data_ptr, data_len, errmsg);
    if(rc<0)
        return rc;
    
    //发送数据
    rc=msglink_send(svr_link->so, (unsigned char *)&msg_buff, msg_buff.head.data_len+sizeof(msg_buff.head), errmsg);
    if(rc<0)//!< 发送数据错误
    {
        if(NULL!=errmsg) 
            sprintf(errmsg, "bu=>cc, 发送业务功能注册失败,rc=[%d]", rc);
        return -2;
    }
    
    return 0;
}