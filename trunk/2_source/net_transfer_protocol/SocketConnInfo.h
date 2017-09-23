#ifndef __MTRX_SOCKET_CONN_INFO_H__
#define __MTRX_SOCKET_CONN_INFO_H__

//#include <map>
//#include <time.h>
#include <sys/time.h>

//定义网络连接统计信息
typedef struct _st_socket_conn_statics
{
    int scs_recv_netpack_count;//接收协议包计数器
    int scs_snd_netpack_count;//发送协议包计数器

    //队列负载(队列中待处理协议包个数)
    int scs_avg_load;//队列平均负载;
    int scs_max_load;//队列最大负载;
    int scs_cur_laod;//队列当前负载;
    int scs_support_max_load;//支持的最大负载
    
    //协议包处理时间,统计协议包request/response的耗时
    int scs_max_cost_time;//最大耗时时间,单位:毫秒
    int scs_avg_cost_time;//平均耗时时间,单位:毫秒
    int scs_cur_cost_time;//最近若干协议包处理耗时时间,单位:毫秒
    
}ST_SocketConnStatics;

//TCP/UDP标记定义
#define SCI_CONN_TYPE_TCP 0 //tcp
#define SCI_CONN_TYPE_UDP 1 //udp

//client/server标记
#define SCI_CS_FLAG_FLAG   0 //客户端
#define SCI_CS_FLAG_SERVER 1 //服务端

//连接状态定义
#define SCI_CONN_STATUS_ESTB       1  //连接已建立
#define SCI_CONN_STATUS_CLOSED     2  //连接已关闭
#define SCI_CONN_STATUS_FAILED     3 //连接错误

//定义网络连接相关信息
typedef struct _st_socket_conn_info
{
   int sci_id;//连接id,可以是socket也可以是自定义的id
   int sci_conn_type; //tcp/udp标记 0-TCP; 1-UDP
   
   long sci_recv_last_net_serial_no;//该连接收到的上一个报文序号(用于确定接收的数据包是否存在丢失);
   long sci_snd_last_net_serial_no; //该连接发送上一个报文序号;

   int sci_conn_timeout;//连接超时时间,如果超过该时间未收到数据,可认为该连接超时,可断开或删除连接;

   //tcp特有
   int sci_sock_fd; //连接socket,TCP有效
   int sci_cs_flag;//client/server标记; 0-client; 1-socket; -1-无效;  请求发起方作为client;另一方作为server
   
   //udp特有信息
   //unsigned char sci_ip[16];//另一方的ip/组播/广播
   //int           sci_port;  //另一方接收服务的端口;
   
   //连接的本地端口和远程端口
   unsigned char sci_remote_ip[16];
   unsigned int sci_remote_port;
   unsigned char sci_local_ip[16];
   unsigned int sci_local_port;
   
   struct timeval sci_start_time;//连接建立时间 //格林尼治时间; 1970-1-1 00:00:00.000
   struct timeval sci_last_recv_time;//上一个协议包接收时间
   struct timeval sci_last_snd_time;//上一个协议包发送时间

   ST_SocketConnStatics sci_conn_statics;//该连接统计信息;

   //连接状态: 
   unsigned char sci_conn_status;
   //协议包接收队列、发送队列、推送队列,考虑到每个连接维护一个,有点难处理,就公用一个队列,同时定义队列中待处理协议包的上线即可;
   //问题:如果达到上限,该怎么处理 丢弃 or 其他 ???
   //
}ST_SocketConnInfo;

//根据文件描述符分配一个连接id
//没有想到很好的方案,使用文件描述符吧
int get_conn_id(int so) { return so; }

#endif
