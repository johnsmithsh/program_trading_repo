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

#include <stdint.h>

//定义业务类型
#define MSGTYPE_CONN        0x45 //!< 建立连接    控制中心<=业务进程
#define MSGTYPE_REG_FUNC    0x46 //!< 注册业务id  控制中心<=业务进程
#define MSGTYPE_DISCONN     0x47 //!< 断开连接    
#define MSGTYPE_HEARTBEAT   0x48 //!< 心跳
#define MSGTYPE_DATA        0x49 //!< 数据传输

#define MSGTYPE_REQ_REQUEST     0x50 //!< 业务请求   控制中心=>业务进程,控制中心主动分配任务
#define MSGTYPE_ACK_REQUEST     0x51 //!< 请求确认包 控制中心<=业务进程
#define MSGTYPE_REQ_RESPONSE    0x52 //!< 请求应答包 控制中心<=业务进程,业务进程处理完成后主动返回处理结果;
#define MSGTYPE_REQ_TRANSFER    0x53 //!< 传送数据请求
#define MSGTYPE_GET_NEXT        0x54 //!< 下一个数据

#define MSGTYPE_GET_TASK         0x56 //!< 请求分配任务 控制中心<=业务进程, 业务进程空闲,主动向控制中心获取任务;
#define MSGTYPE_ASSIGN_TASK      0x57 //!< 分配任务     控制中心=>业务进程
#define MSGTYPE_NOTIFY_TASK_OVER 0x58 //!< 任务完成     控制中心<=业务进程, 通知控制中心,任务完成;

//
#define MSG_BUNO_INVALID   (-1) //无效buno

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

#define C_YES '1' 
#define C_NO  '0'

#define MASKHEAD_REQ       (1<<0) //请求应答标记;0-请求; 1-应答
#define MASKHEAD_RSP       (1<<1) //请求应答标记;0-请求; 1-应答
#define MASKHEAD_ACK       (1<<2)  //ack报文
#define MASKHEAD_PUSH      (1<<3)  //推送报文

//消息头
typedef struct __st_msg_head
{
    unsigned int   data_len;//!< 数据包长度(不包含数据头)
    unsigned short msgid;//!< 业务类型
	 unsigned int   mask;//!<掩码,见MASKHEAD_XXX定义
    unsigned char  ccflag;//!< 控制标记, 保留
    unsigned char  headflag;//!< 报文头标记, 保留
    unsigned int   req_id; //!< 保留
    unsigned int   rsp_id; //!< 保留
	 unsigned int   ack_id; //!< 保留
	 unsigned int   head_crc;//报文头crc校验
}ST_MSG_HEAD;

typedef struct //!< 报文通用部分,每个报文都包含; 当然建立连接请求中不能包含所有信息
{
     //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
    char          group_no[16];//!< 业务组号
    unsigned int  bu_no;       //!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
    unsigned int  bcc_id;      //!< 控制中心id

    unsigned int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
    unsigned int  msg_num;   //!< 消息个数
    char session_mode;  //!< 请求任务类型
    char mask;      //!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
}ST_MSG_COMMON;

//一个msg消息包
typedef struct
{
	ST_MSG_HEAD   head;//报文头
	ST_MSG_COMMON commoninfo;//msg消息说明信息
   char data_buff[8192];//msg消息数据
}ST_MSGLINK_BUFF;


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
 
	char prog_name[32];//!< 程序名
   unsigned long proc_id;//!< 进程号
	char version[4];//!< 版本信息
	
	char mode;//!< 模式
}MSG_REQ_CONN;

typedef struct //!< 连接应答: 控制中心 => 业务进程
{
    int  bu_no; //控制中心分配给业务的id,每个服务进程都不一样
    int  bcc_id;//控制中心id
    char if_succ;
    char szmsg[255];
}MSG_ANS_CONN;

//-------------------------------------------------------------
//建立连接报文 MSGTYPE_DISCONN
typedef struct //!< 断开连接请求: 业务进程 => 控制中心
{
    char group_no[16];//!< 业务组号
    char group_desc[64];//!< 业务组描述
    char srv_name[64];//!< 服务名,一般设置为程序名即可
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
//----------------------------------------------------------------------------------------------------------
//通信类型
#define MSGLINK_COMMTYPE_REQRSP     '1'  //请求-应答模型
#define MSGLINK_COMMTYPE_REQRSPPUSH '2'  //请求-应答-推送模型
#define MSGLINK_COMMTYPE_PUSH       '3'  //推送
#define MSGLINK_COMMTYPE_FORWARD    '4'  //转发
//注册功能报文
typedef struct //!< 注册业务请求: 控制中心 <= 业务进程
{
    char bu_func_id[16];  //!< 业务进程支持的业务id
    char bu_func_desc[64];//!< 业务功能说明
    char bu_name[32];     //!< 业务名

    int  priority;        //!< 优先级;
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
#define MSK_ACK_BIT   2 //ACK标记
#define MSK_PUSH_BIT  3 //推送消息

////分配任务报文
//typedef struct //!< 分配任务请求: 控制中心 => 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的报文请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    unsigned char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//}MSG_REQ_REQUEST;
//
//typedef struct //!< 分配任务应答: 控制中心 <= 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//	
//	char if_succ;
//    char szmsg[255];
//}MSG_ACK_REQUEST;
////-----------------------------------------------------------
////业务完成返回应答结果报文
//typedef struct //!< 任务返回结果请求: 控制中心 <= 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//}MSG_REQ_RSP;
//
//typedef struct //!< 任务返回结果应答: 控制中心 => 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//	
//	char if_succ;
//    char szmsg[255];
//}MSG_ACK_RSP;
////----------------------------------------------------------
////业务数据推送报文
//typedef struct //!< 推送数据请求: 控制中心 <= 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//}MSG_REQ_TRANSFER;
//
//typedef struct //!< 任务返回结果应答: 控制中心 => 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//	
//	char if_succ;
//    char szmsg[255];
//}MSG_ACK_TRANSFER;
////-----------------------------------------------------------
////业务数据推送报文
//typedef struct //!< 推送数据请求: 控制中心 <= 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//}MSG_REQ_PUSH;
//
//typedef struct //!< 推送应答: 控制中心 => 业务进程
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//}MSG_ANS_PUSH;
////-------------------------------------------------------------
////任务转发报文: 当前中心负责把请求转发到指定中心,后续应答不再负责; 只会出现在控制中心与控制中心之间
//typedef struct //!< 分配任务请求: 控制中心 => 控制中心
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag; forward_flag;
//}MSG_REQ_FORWARD;
//
//typedef struct //!< 分配任务应答: 控制中心 <= 控制中心
//{
//    //!< 业务进程信息说明,用于业务进程校验是否属于本服务进程
//    char group_no[16];//!< 业务组号
//    int  bu_no;//!< 业务服务id; 建立连接时,控制中心分配给业务的id,每个服务进程都不一样
//    int  bcc_id;//!< 控制中心id
//    
//    int  request_id;//!< 控制中心生成的请求id,需要在应答域回写该字段
//    int  msg_num;//!< 消息个数
//    char req_mode;//!< 请求任务类型
//    char mask;//!< first_flag; next_flag; req_flag; rsp_flag; push_flag;
//}MSG_ACK_FORWARD;
//-----------------------------------------------------------------------------------------------


typedef struct //!< ACK消息
{
   char if_succ;
	char szmsg[255];
}MSG_RSP;

typedef struct //!< ACK消息
{
	char if_succ;
	char szmsg[255];
}MSG_ACK;





typedef struct __st_bufunc_item
{
    char func_id[16];  //!< 功能id 
    char func_type;    //!< 1-请求-应答; 1-请求,应答,推送; 2-推送; 3-转发;
    char func_desc[64];//!< 业务功能说明
}ST_BUFUNC_ITEM;
#endif
