#ifndef __MXX_SVR_LINK_H_
#define __MXX_SVR_LINK_H_

#include <stddef.h>
#include "msg_link_define.h"


//-------------------------------------------------------------------------------------------------------------------------------------------------
//定义连接信息,描述控制中心与业务进程之间的连接
typedef struct
{
    //业务组信息
   char group_no[16];//!< 业务进程自己定义,不能重复
	char group_desc[64];
   int  bu_no; //!< 控制中心为业务进程分配

   int  bcc_id;

   int  link_mode;//!< 连接模式 bu对bu, bcc对bcc, bcc对bu
   int  link_role; //!< 连接角色 业务服务角色, 控制中心角色;
}ST_LINK_INFO;

//连接句柄
typedef struct
{
   ST_LINK_INFO link_info;
	int so;
	int start_timestamp;//时间戳

	unsigned int send_serial;//!< 发送请求序号,每次递增1
	unsigned int recv_serial;//!< 接收序号,即对方发送序号

	char version[4];//协议版本号
}ST_SVR_LINK_HANDLE;

typedef void* SVRLINK_HANDLE;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//链接句柄相关操作
SVRLINK_HANDLE svrhandle_open();
void           svrhandle_close(SVRLINK_HANDLE &svrlinkhandle);
int            svrhandle_init(SVRLINK_HANDLE svrlinkhandle);
int            svrhandle_set_groupinfo(SVRLINK_HANDLE svrlinkhandle, const char *group_no, const char *group_desc, const char *group_version, int pid);
int            svrhandle_set_linkinfo(SVRLINK_HANDLE  svrlinkhandle, unsigned int bcc_id, unsigned int bu_no);
int            svrhandle_set_socket(SVRLINK_HANDLE    svrlinkhandle, int so);
unsigned int   svrhandle_next_serial(SVRLINK_HANDLE   svrlinkhandle);

int            svrhandle_get_socket(SVRLINK_HANDLE    svrlinkhandle);
ST_SVR_LINK_HANDLE *svrhandle_to_linkinfo(SVRLINK_HANDLE    svrlinkhandle);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//报文接收与发送数据相关操作;
//这些操作默认通过阻塞方式实现;
#define MSG_SOCK_NO_DATA  -101 //没有数据
#define MSG_SOCK_CLOSE    -102 //文件描述符关闭
#define MSG_SOCK_INTR     -103 //读取过程被中断
#define MSG_HEAD_INVALID  -104 //报文头无效
#define MSG_BUFF_ENOMEM   -105 //内存不足
#define MSG_SOCK_RETRY    -106 //超过重试次数
#define MSG_SOCK_HALFPACK -107 //发送或接收半个数据包

#define EINVLID_ARG       -9003 //参数无效
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
int msglink_send(int so, const unsigned char *buff, size_t data_len, int *send_len, char *szmsg);

/**
 * @brief 接收数据; 要么接收一个完整数据包,要么返回错误
 *     阻塞接收,调用该函数前请确认socket存在数据;
 *     默认从报文头开始接收
 * @param
 *   [in]so: socket
 *   [out]buff,buffsize: 接收缓冲区及缓存区大小
 *   [out]recv_len: 接收数据长度
 *   [out]szmsg: 错误消息
 * @retval >=0-返回接收字节数;
 *   MSG_BUFF_ENOMEM/MSG_SOCK_CLOSE/MSG_SOCK_NO_DATA; <0-其他错误;
 * @note
 *     需要校验接收数据长度与数据报文长度是否匹配
 */
int msglink_recv(int so, unsigned char *buff, size_t buffsize, int *recv_len, char *szmsg);

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
int msglink_peekhead(int so, ST_MSG_HEAD *msg_head_ptr, char *szmsg);

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
int msglink_recvdata(int so, unsigned char *buff, size_t datalen, int *recv_len, char *szmsg);

//@brief 接收消息数据 需要使用者填充报文头
int svrlink_recv(SVRLINK_HANDLE link_handle,  ST_MSGLINK_BUFF *recv_buff, char *errmsg);
//@brief 发送消息数据 需要使用者填充报文头
int svrlink_send(SVRLINK_HANDLE link_handle,  ST_MSGLINK_BUFF *send_buff, char *errmsg);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief 创建连接句柄; NULL-创建失败;
 *  @param
 *    [in]group_no: 组号; 业务进程存在组号; 控制中心没有组号;
 *    [in]group_desc: 业务进程的业务说明;
 *    [in]group_version: 组号版本信息说明
 *    [in]pid: 进程号
 * @retval
 *     连接句柄; NULL-创建失败;
 **/
SVRLINK_HANDLE svrlink_create(const char *group_no, const char *group_desc, const char *group_version, int pid);

//@brief 删除svrlink_create创建的句柄; 0-成功; <0-失败;
int svrlink_close(SVRLINK_HANDLE svrlinkhandle);

/** @brief 设置业务进程连接信息
 *  @param
 *    [in]group_no: 组号; 业务进程存在组号; 控制中心没有组号;
 *    [in]group_desc: 业务进程的业务说明;
 *    [in]group_version: 组号版本信息说明
 *    [in]pid: 进程号
 * @retval
 *     0-成功; <0-失败;
 **/
int svrlink_setbuinfo(SVRLINK_HANDLE svrlinkhandle, const char *group_no, const char *group_desc, const char *group_version, int pid);

/** @brief 设置业务号(控制中心分配)
 *  @param
 *    [in]bu_no: bu_id; 控制中心分配
 * @retval
 *     0-成功; <0-失败;
 **/
int svrlink_setbuno(SVRLINK_HANDLE svrlinkhandle, int bu_no);

/** @brief 设置控制中心信息
 *  @param
 *    [in]bcc_id: 组号; 业务进程存在组号; 控制中心没有组号;
 *    [in]bcc_version: 业务进程的业务说明;
 * @retval
 *     0-成功; <0-失败;
 **/
int svrlink_setbccinfo(SVRLINK_HANDLE svrlinkhandle, int bcc_id);

//功能: 服务端-启动监听服务
int svrlink_listen(SVRLINK_HANDLE link_handle, char *ip, int port, int bcc_id);

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
int svrlink_connect(SVRLINK_HANDLE svrlinkhandle,  char *ip, int port, char *errmsg);
int svrlink_ans_connect(SVRLINK_HANDLE svrlinkhandle, unsigned int bcc_id, unsigned int bu_no, char if_succ, const char *szmsg, char *errmsg);

int svrlink_ack(SVRLINK_HANDLE svrlinkhandle, ST_MSG_HEAD *msg_head_ptr, char if_succ, const char *szMsg, char *errmsg);

// @brief 断开连接 MSGTYPE_DISCONN
int svrlink_disconnect(SVRLINK_HANDLE svrlinkhandle, char *errmsg);
int svrlink_ans_disconnection(SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg);

//@brief 客户端=>控制中心: 注册业务功能号
//     MSGTYPE_REG_FUNC
int svrlink_register_function(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg);
int svrlink_ans_register_function(SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg);

int svrlink_heatbeat(SVRLINK_HANDLE svrlinkhandle, char *errmsg);
int svrlink_ans_heatbeat(SVRLINK_HANDLE svrlinkhandle, char if_succ, const char *szmsg, char *errmsg);

//@brief 请求应答 MSGTYPE_REQ_REQUEST
int svrlink_request(SVRLINK_HANDLE svrlinkhandle, unsigned char *data, int data_len, char *szMsg);
//int svrlink_response(SVRLINK_HANDLE link_handle, unsigned char *data, int data_len, char *szMsg);
//
//@brief 发送确认数据
//int svrlink_ack(SVRLINK_HANDLE link_handle, unsigned char *data, int data_len, char *szMsg);
//
//@brief 推送数据
//int svrlink_push(SVRLINK_HANDLE link_handle, unsigned char *data, int data_len, char *szMsg);


//功能:如果数据比较多,需要使用这种方式
//@brief 发送数据,建议每次传输多个完整的业务报文;
//    MSGTYPE_DATA
int svrlink_transfer(SVRLINK_HANDLE svrlinkhandle, unsigned char *data_ptr, int data_len, bool first_flag, bool next_flag, char *errmsg);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
