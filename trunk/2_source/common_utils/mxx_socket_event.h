#ifndef __MXX_SOCKET_EVENT_H_
#define __MXX_SOCKET_EVENT_H_

#define MXX_SUCC_CODE  0
#define MXX_ERROR_SOCKET_RECV_FAILED -1000//socket读取失败,错误码从errno获取
#define MXX_ERR_CONN_CLOSE  -1001 //socket连接被对方关闭
#define MXX_ERR_BIN_PACK_HEAD_UNCOMPLETE  -1002  //报文头不完整
#define MXX_ERR_BIN_PACK_HEAD_TOSMALL    -1003  //报文长度过短
#define MXX_ERR_BIN_PACK_HEAD_TOLONG     -1004  //报文长度过长
#define MXX_ERR_BIN_PACK_ALLOC_FAILED    -1005  //申请缓存失败
#define MXX_ERR_BIN_PACK_UNCOMPLETE      -1006 //报文不完整,即报文头指定的长度大于实际读取到的长度
#define MXX_ERR_BIN_PACK_UNVALID         -1007  //报文无效,未通过校验
#define MXX_ERR_TIMEOUT         -1008  //超时

#define MXX_SEND_ERROR   -1009 //发送超时
#define MXX_SEND_BUFF_FULL  -1010 //socket发送缓冲区已满

#include "mxx_bin_pack.h"

//业务上每次读一个完整协议包后才会从推送到缓存队列;
//      每次写一个完整协议包后才会从发送队列获取下一个发送协议包;
//   为安全起见,每个完整的协议包必须是格式: "长度+数据"
//采用异步io读写时,每次收到读写事件通知,需要判断当前协议包处理到哪一步了
typedef struct __st_buff_info
{
   ST_BIN_BUFF *buff;//指向一个完整的协议包缓存;
   int buff_size;//缓存总大小
   int data_len;//当前协议包数据数据长度(不含报文头长度)
   int total_proc_len;//已经处理数据长度
}ST_SOCKET_BUFF_INFO;




//收到读数据事件处理函数;
int mxx_read_event(int so, ST_SOCKET_BUFF_INFO *recv_buff_info);

//收到写数据事件处理函数
int mxx_write_event(int so,ST_SOCKET_BUFF_INFO *snd_buff_info);

//校验协议包是否完整;
int mxx_check_data_intergrality(ST_BIN_BUFF *pack);

#endif

