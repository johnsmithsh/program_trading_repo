/*
 * 控制中心线程
 * 
 *  该线程解析接收线程的数据包,解析后进行处理
 * 
 */

#include "ctrl_center_thread.h"
#include "thread_recv.h"
#include "mxx_bin_pack.h"

#include "log.h" //日志


#include "mxx_net_socket.h"
#include "mxx_socket_event.h"
#include "itc_mutex.h"
#include "SocketConnInfo.h"
#include "SocketConnPool.h"
#include "os_time.h"


#include <deque>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

using namespace std;


#define TEST_MSG printf

////////////////////////////////////////////////////////////////////////
//构造函数
CCtrlCenterThread::CCtrlCenterThread(const char *thread_name/*="recv_thread"*/):Thread_Base(thread_name)
{
   //m_server_port=0;
}
//析构函数
CCtrlCenterThread::~CCtrlCenterThread()
{
}

int CCtrlCenterThread::init()
{
   return 0;
}

int CCtrlCenterThread::terminate()
{
   return 0;
}

//处理接收缓存(队列)中的数据
int CCtrlCenterThread::handle_routine() 
{
  int rc;
  ST_BIN_BUFF *bin_buff;
  char *data_ptr=NULL;//数据指针
  int data_len=0;
  for(;;)
  {
     //从接收缓存获取协议包
     bin_buff=recv_que_pop();
     if(NULL==bin_buff)
     {
        sleep(3);
        continue;
     }

     //从协议包获取数据指针
     data_ptr=(char *)mxx_bin_pack_data_ptr(bin_buff);
     data_len=mxx_bin_pack_datalen(bin_buff);
     if(NULL==data_ptr)
     {
         TEST_MSG("该数据包没有数据\n");
         continue;
     }

     //处理数据
     printf("Debug: data_len=[%d], msg=[%s]\n", data_len, data_ptr);

     //释放
     mxx_free_bin_pack(bin_buff);
     bin_buff=NULL;
  }

  return 0;
}
