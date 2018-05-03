#ifndef __MXX_SOCKET_HANDLE_H_
#define __MXX_SOCKET_HANDLE_H_
#include "SocketConnInfo.h"
#define MAX_IP_LEN 64 //!< ip地址最大长度

//每个数据结构表示一个connect连接,包括so,状态,该连接统计信息
typedef struct {
  int fd;                           // 连接句柄
  char host[MAX_IP_LEN];           // IP地址
  int port;                         // 端口

  bool  status;                     // 使用标记; true-正在使用; false-没有使用;
  
  ST_SocketConnInfo conn_info;//连接信息

  //在该level socket发送必须串行,发送同样如此
  //发送与接收可以并行,故需要发送缓存与接收缓存两个;
  //ST_SOCKET_BUFF_INFO recv_buff_info;//接收缓存info
  //ST_SOCKET_BUFF_INFO snd_buff_info; //发送缓存info
} ST_SockConnHandle;//socket连接句柄

#endif
