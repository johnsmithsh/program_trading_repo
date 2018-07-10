#ifndef __MXX_SVR_LINK_H_
#define __MXX_SVR_LINK_H_

typedef void* SVRLINK_HANDLE;

//@brief 创建连接句柄; 返回连接句柄; NULL-创建失败;
SVRLINK_HANDLE svrlink_create();

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

// @brief 断开连接 MSGTYPE_DISCONN
int svrlink_disconnect(SVRLINK_HANDLE link_handle);

//@brief 客户端=>控制中心: 注册业务功能号
//     MSGTYPE_REG_FUNC
int svrlink_register_function(SVRLINK_HANDLE svrlinkhandle, char *register_info, size_t len, char *errmsg);


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

//brief 发送与接收消息数据 需要使用者填充报文头
int svrlink_recv(SVRLINK_HANDLE link_handle,  ST_MSGLINK_BUFF *recv_buff, char *errmsg);
int svrlink_send(SVRLINK_HANDLE link_handle,  ST_MSGLINK_BUFF *send_buff, char *errmsg);
#endif