/*
 * myepoll.cpp
 *
 * Created on: 2013-06-03
 * Author: liuxiaoxian
 * 提高ms并发度调研：把客户端发来的数据发过去
 */

//#include <sys/socket.h>
//#include <sys/epoll.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
//#include <iostream>

#include <string.h>
#include <stdlib.h>

#include "log.h" //日志


#include "mxx_net_socket.h"
#include "mxx_socket_event.h"

#include "thread_recv.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//系统版本信息
const char g_system_version[]="0.0.0.1";
void print_version()
{
   printf("version information:\n"
          "   name:    mxx service\n"
          "   version: %s\n"
          "   build:   %s %s\n\n", g_system_version, __DATE__, __TIME__);
}
/////////////////////////////////////////////////////////////////////////////

CRecvThread g_recv_thread;//接收线程
int main(int argc, char **argv) 
{
  int rc;

  //socket信息
  unsigned short port = 12345;
  //unsigned int so_concurrency=100;//连接并发数

  //日志信息 
  char log_file_path[256]={"log/"};//日志文件路径
  char log_file_prefix[64]={0};//日志文件名前缀
  
  //epoll配置信息
  int epoll_max_size=1000; //epoll句柄监听最大文件数
  int epoll_wait_timeout=5000;//epoll_wait超时时间,单位毫秒; 0-立即返回; -1不确定;

  print_version();

  //解析命令行参数
  //if(argc == 2){
  //    port = atoi(argv[1]);
 // }

  //读取ini配置文件
  //

  //初始化日志文件
  printf("init logfile [%s]...\n", log_file_path);
  CLogFileManage *logfile=log_file_init(log_file_path, NULL);
  if(NULL==logfile)
  {
    printf("FATAL: 初始化日志[%s]信息失败!\n", log_file_path);
    return -3;
  }
  printf("succ to init log file\n");

  //启动客户端接收线程
  INFO_MSG("start thread [recv_client_request]...");
  g_recv_thread.set_thread_name("recv_client_request");
  g_recv_thread.start();//启动线程
  INFO_MSG("succed to start thread");
  
  while(1)
  {
     sleep(2);
  }

  //关闭各个线程
  INFO_MSG("stop all thread...");
  INFO_MSG("end of stop thread");

  //关闭日志
  INFO_MSG("close log file...");

  return 0;
}
