/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义控制中心与业务服务通信消息格式
 *@details 
 *Others:  
 *Function List:  
         1.………… 
         2.………… 
 *@note  
 *@history:  
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/  
#ifndef _MXX_MSG_LINK_DEFINE_H_
#define _MXX_MSG_LINK_DEFINE_H_

//定义业务类型
#define MSGTYPE_CONN        0x45 //!< 建立连接
#define MSGTYPE_REG_FUNC    0x46 //!< 注册业务id
#define MSGTYPE_DISCONN     0x47 //!< 断开连接
#define MSGTYPE_HEARTBEAT   0x48 //!< 心跳
#define MSGTYPE_DATA        0x49 //!< 数据传输
#define MSGTYPE_REQ_REQUEST     0x50 //!< 业务请求
#define MSGTYPE_ACK_REQUEST     0x51 //!< 请求确认包
#define MSGTYPE_REQ_ESP         0x52 //!< 请求应答包
#define MSGTYPE_REQ_TRANSFER    0x53 //!< 传送数据请求


//定义掩码
#define BM_REQ_RSP  0 //!< 请求包 0-请求包; 1-应答包;
#define BM_PUSH     1 //!< 推送   0-普通数据包; 1-推送;
#define SYN_MASK 3 //
#define ACK_MASK 4 //ack标记位
#define PUSH_MASK 5 //推送标记位
//-------------------------------------------------------------
//MLHF: Msg Link Head Flag
//参考TCP报文标志位
//ISN //!<初始序号
/*
//#define MLHF_URG_BIT  0 //!< 紧急数据有效位
//#define MLHF_SYN_BIT   1 //!< 同步序号,连接请求;
//#define MLHF_ACK_BIT   2 //!< 确认位,报文包含确认号
//#define MLHF_FIN_BIT   3 //!< 释放连接
//#define MLHF_PSH_BIT   4 //!< push位,指示接收方应该尽快将这个报文段交给应用层而不用等待缓冲区装满
//#define MLHF_RSP_BIT   5 //?? 
//#define MLHF_RST_BIT   6 //!< 连接重置位,表示数据包已经乱了;用于复位由于主机崩溃或其他原因而出现错误的连接.
                         //!< 它还可以用于拒绝非法的报文段和拒绝连接请求.
                         //!< 一般情况下,如果收到一个 RST 为 1 的报文,那么一定发生了某些问题
*/

#define MSGLINK_YES '1' 
#define MSGLINK_NO  '0'

//消息头
typedef struct __st_msg_head
{
    unsigned int msgid;//!< 业务类型
	short        data_len;//!< 数据包长度不包含数据头
    char ccflag;//!< 控制标记
    char headflag;//!< 报文头标记
    int req_id; //!<
    int rsp_id;
}ST_MSG_HEAD;

//应答消息
typedef struct __st_msg_rsp
{
    char if_success;//!< 成功标记
    char err_msg[255]; //!< 错误消息
}ST_RSP_MSG;
//-------------------------------------------------------------
//定义初始化连接(创建连接)报文 MSGTYPE_CONN
typedef struct //!< 连接请求: 业务进程 => 控制中心
{
    char group_no[16];//!< 业务组号
    char group_desc[64];//!< 业务组描述
    char mode;//!< 模式 
    char version[4];//!< 版本信息
    long proc_id;//!< 进程号
}MSG_REQ_CONN;

typedef struct //!< 连接应答: 控制中心 => 业务进程
{
    int bu_no; //控制中心分配给业务的id,每个服务进程都不一样
    int bcc_id;//控制中心id
    char if_succ;
    char szmsg[255];
}MSG_ANS_CONN;

//-------------------------------------------------------------
//建立连接报文 MSGTYPE_DISCONN
typedef struct //!< 断开连接请求: 业务进程 => 控制中心
{
    char group_no[16];//!< 业务组号
    char group_desc[64];//!< 业务组描述
    int  bu_no;//建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//控制中心id
}MSG_REQ_DISCONN;

typedef struct //!< 断开连接应答: 控制中心 => 业务进程
{
    int bu_no;//控制中心分配给业务的id,每个服务进程都不一样
    int bcc_id;//控制中心id
    
    char if_succ;
    char szmsg[255];
}MSG_ANS_DISCONN;
//-------------------------------------------------------------
//注册功能报文
typedef struct //!< 注册业务请求: 控制中心 <= 业务进程
{
    char bu_func_id[16];//!< 业务进程支持的业务
    char bu_func_desc[63];//!< 业务功能说明
    char bu_func_type;    //!< 1-请求-应答; 1-请求,应答,推送; 2-推送; 3-转发;
}MSG_REQ_REGFUNC;

typedef struct //!< 注册业务应答: 控制中心 => 业务进程 
{
    char if_succ;
    char szmsg[255];
}MSG_ANS_REGFUNC;
//-------------------------------------------------------------
//定义控制位
#define MSK_FIRST_BIT 0 //第一个报文
#define MSK_NEXT_BIT  1 //是否存在下一个报文; 用于控制传送数据过长的情况
#define MSK_PUSH_BIT  2 //推送消息

//分配任务报文
typedef struct //!< 分配任务请求: 控制中心 => 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的报文请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    unsigned char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_REQ_REQUEST;

typedef struct //!< 分配任务应答: 控制中心 <= 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
	
	char if_succ;
    char szmsg[255];
}MSG_ACK_REQUEST;
//-----------------------------------------------------------
//业务完成返回应答结果报文
typedef struct //!< 任务返回结果请求: 控制中心 <= 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_REQ_RSP;

typedef struct //!< 任务返回结果应答: 控制中心 => 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
	
	char if_succ;
    char szmsg[255];
}MSG_ACK_RSP;
//----------------------------------------------------------
//业务数据推送报文
typedef struct //!< 推送数据请求: 控制中心 <= 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_REQ_TRANSFER;

typedef struct //!< 任务返回结果应答: 控制中心 => 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
	
	char if_succ;
    char szmsg[255];
}MSG_ACK_TRANSFER;
//-----------------------------------------------------------
//业务数据推送报文
typedef struct //!< 推送数据请求: 控制中心 <= 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_REQ_PUSH;

typedef struct //!< 推送应答: 控制中心 => 业务进程
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_ANS_PUSH;
//-------------------------------------------------------------
//任务转发报文: 当前中心负责把请求转发到指定中心,后续应答不再负责; 只会出现在控制中心与控制中心之间
typedef struct //!< 分配任务请求: 控制中心 => 控制中心
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag; forward_flag;
}MSG_REQ_FORWARD;

typedef struct //!< 分配任务应答: 控制中心 <= 控制中心
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;//!< 消息个数
    char req_mode;//!< 请求任务类型
    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_ACK_FORWARD;
//-----------------------------------------------------------------------------------------------
typedef struct //!< 报文通用部分,每个报文都包含; 当然建立连接请求中不能包含所有信息
{
    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char group_no[16];//!< 业务组号
    int  bu_no;       //!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;      //!< 控制中心id
    
    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    int  msg_num;   //!< 消息个数
    char req_mode;  //!< 请求任务类型
    char mask;      //!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}MSG_COMMON;

typedef struct //!< ACK消息
{
    char if_succ;
	char szmsg[255];
}MSG_ACK;

//-----------------------------------------------------------
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

//
typedef struct
{
    ST_SVR_LINK_INFO link_info;
	int so;
	int start_time;
	
	unsigned int send_serial;//!< 发送请求序号,每次递增1
	unsigned int recv_serial;//!< 接收序号,即对方发送序号
	
	char version[4];//协议版本号
	
}ST_SRV_LINK_HANDLE;

typedef struct
{
    ST_MSG_HEAD   head;
	ST_MSG_COMMON commoninfo;
    char data_buff[8192];
}ST_MSGLINK_BUFF;

typedef struct __st_bufunc_item
{
    char func_id[16];  //!< 功能id 
    char func_type;    //!< 1-请求-应答; 1-请求,应答,推送; 2-推送; 3-转发;
    char func_desc[64];//!< 业务功能说明
}ST_BUFUNC_ITEM;
#endif