/*
 * myepoll.cpp
 *
 * Created on: 2013-06-03
 * Author: liuxiaoxian
 * 提高ms并发度调研：把客户端发来的数据发过去
 */

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

#include <string.h>
#include <stdlib.h>

#include "log.h" //日志

//#include "lxx_net.h"

#include "mxx_net_socket.h"
//#include "mxx_socket_event.h"

using namespace std;

#define MAX_EPOLL_SIZE 500 //epoll句柄监听最大连接数

#define MAX_CLIENT_SIZE 500
#define MAX_IP_LEN      16
#define MAX_CLIENT_BUFF_LEN 1024
#define QUEUE_LEN 500
#define BUFF_LEN 1024
int fd_epoll = -1;
int fd_listen = -1;


//打印测试消息
#define TEST_MSG printf

typedef struct __st_buff_info
{
   char *buff;//指向一个完整的协议包缓存;
   int buff_size;//缓存总大小
   int data_len;//缓存数据长度
   int total_proc_len;//已经处理数据长度
}ST_SOCKET_BUFF_INFO;


//客服端连接
typedef struct {
  int fd;                           // 连接句柄
  char host[MAX_IP_LEN];           // IP地址
  int port;                         // 端口
  int len;                          // 缓冲区数据大小
  char recv_buff[16];   // 缓冲数据
  char snd_buff[MAX_CLIENT_BUFF_LEN];    // 发送缓存
  bool  status;                     // 状态

  //在该level socket发送必须串行,发送同样如此
  //发送与接收可以并行,故需要发送缓存与接收缓存两个;
  ST_SOCKET_BUFF_INFO recv_buff_info;//接收缓存info
  ST_SOCKET_BUFF_INFO snd_buff_info; //发送缓存info
} client_t;

client_t *ptr_cli = NULL;

//功能:申请一个空闲网络连接
client_t *alloc_conn_info()
{
   int i=0;
   client_t *ptr=NULL;
   for(i=0; i<MAX_CLIENT_SIZE; i++)
   {
     ptr=ptr_cli+i;
     if(ptr->status)
        continue;
     
     memset(&ptr_cli[i], 0, sizeof(client_t));
     ptr->status=true;
     return ptr;
   }

   return NULL;
}

//功能:释放网络连接
void free_conn_info(client_t *ptr)
{
   if(NULL==ptr) return;
   memset(ptr, 0, sizeof(client_t));
   ptr->status=false;
   return;
}

//功能: 根据socket文件描述符找到对应的连接信息;
client_t *get_conn_info(int so)
{
   int i=0;
   client_t *ptr=NULL;
   for(i=0; i<MAX_CLIENT_SIZE; i++)
   {
     ptr=ptr_cli+i;
     if( (ptr->fd==so) && (ptr->status) )
        return ptr;
   }
   return NULL;
}


//功能: socket加入到监听队列; 
//参数:
//  [in]fd_epoll监听句柄; 
//  [in]fd:socket文件描述符; 
//  [in]ev: 监听事件
//返回值: 0-成功; <0-失败;
int epoll_add(int fd_epoll, int fd, struct epoll_event *ev) {
  if (fd_epoll < 0 || fd < 0 || ev == NULL) {
    return -1;
  }

  if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd, ev) < 0) {
    ERROR_MSG("epoll_add failed(epoll_ctl)[fd_epoll:%d,fd:%d][%s]", fd_epoll, fd, strerror(errno));
    return -1;
  }

  INFO_MSG("epoll_add success[fd_epoll:%d,fd:%d]\n", fd_epoll, fd);
  return 0;
}

/*
 * 功能: 将socket从epool监听队列中删除
 * 参数:
 *    [in]fd_epool: epoll句柄
 *    [in]fd: socket文件描述符
 * 返回值: 0-成功; <-0失败;
 */ 
int epoll_del(int fd_epoll, int fd) {
  if (fd_epoll < 0 || fd < 0) {
    return -1;
  }

  struct epoll_event ev_del;
  if (epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd, &ev_del) < 0) {
    ERROR_MSG("epoll_del failed(epoll_ctl)[fd_epoll:%d,fd:%d][%s]\n", fd_epoll, fd, strerror(errno));
    return -1;
  }
  //close(fd);
  INFO_MSG("epoll_del success[epoll_fd:%d,fd:%d]\n", fd_epoll, fd);
  return 0;
}

// 接收数据
int do_read_data(int so, bool *close_flag) {
  if(NULL!=close_flag) *close_flag=false;

  if (so<=0) {//无效文件描述符
    return -1;
  }
  int n=0;

  //查找socket表示的网络连接;
  client_t *ptr=NULL;
  ptr=get_conn_info(so);
  if(NULL==ptr)
      return -2;

  int count=0;
  int sum_count=0;
  bool is_more_data=true;//是否有更多的数据需要读取; true-有; false-没有;
  //开始读取...
  while(1){
    char *recv_pos=ptr->recv_buff_info.buff + ptr->recv_buff_info.total_proc_len;
    int recv_size = ptr->recv_buff_info.buff_size - ptr->recv_buff_info.total_proc_len;
    if(recv_size>0){//接收缓存区存在可用空间
       TEST_MSG("[%s:%d-%d],index=[%d],total_len=[%d] recv_size=[%d] start recv data...\n", 
                ptr->host, ptr->port, count, (recv_pos-ptr->recv_buff_info.buff), ptr->recv_buff_info.total_proc_len,recv_size);
       n=recv(ptr->fd, recv_pos, recv_size,0);
       if(n<0){
          if((EAGAIN==errno) || (EWOULDBLOCK==errno))//没有可用数据
          {
             TEST_MSG("errno=EAGAIN or EWOULDBLOCK");
          }
          else{//0==n,对方关闭连接; 其他错误;
              TEST_MSG("[%s:%d-%d] The Client closed(read)\n", ptr->host, ptr->port,count);
              //epoll_del(fd_epoll, ptr->fd);
              //ptr->status = false;

              //出现错误,需要关闭socket连接
              if(NULL!=close_flag) *close_flag=true;
          }
          break;
       }
       else if(0==n){//对方已经关闭
          if(NULL!=close_flag) *close_flag=true;
          TEST_MSG("n=0");
          break;
       }
       else if(n==recv_size){//可能还有数据
       }
     
       if(n>0)  
       {
         sum_count+=n;
         ptr->recv_buff_info.total_proc_len+=n;
       }
       TEST_MSG("[%s:%d-%d],end of recv!\n",ptr->host, ptr->port,count);
       if(*close_flag)
       {
          TEST_MSG("===>   n=[%d],errno[%d],close_flag=[true]; recv_msg[%s]\n\n", n,errno, recv_pos);
       }
       else
       {
          TEST_MSG("===>   n=[%d],errno[%d],close_flag=[false]; recv_msg[%s]\n\n", n,errno, recv_pos);
       }
       
    }
    else{//recv_size<=0,缓存不足,无论是否有数据均跳出;
       TEST_MSG("Warning:[%s:%d:%d] recv_buff is full\n", ptr->host, ptr->port,count);
    }
    if(recv_size<=0) //缓存不足,无论存在未读数据都不能继续读数据了;
        break;
    else if(*close_flag)//recv返回值=0,说明对方关闭,不能继续读取
        break;
    else
    {
       if(n<recv_size) //读取的数据比缓存空间小,说明肯定读完了
          break;
       else if(n==recv_size)//可能还存在未读数据
       {
          //不知道是否需要继续读,如果没有数据了,会造成死锁;
       }
       else
          break;
    }
    memset( ptr->recv_buff_info.buff, 0,  ptr->recv_buff_info.buff_size);
    ptr->recv_buff_info.total_proc_len=0;
    
  }

  //校验数据是完整协议包
  //
  bool is_intergrality=true;
  
  if(is_intergrality)
  {
     //完整协议包,则推送到缓存中
     
     memset( ptr->recv_buff_info.buff, 0,  ptr->recv_buff_info.buff_size);
     ptr->recv_buff_info.total_proc_len=0;
  }
  else if(ptr->recv_buff_info.total_proc_len >= ptr->recv_buff_info.buff_size)
  {
     //不是完整协议包,且 缓存区已经填满,则(协议包不应该有这么长)丢弃数据
     memset( ptr->recv_buff_info.buff, 0,  ptr->recv_buff_info.buff_size);
     ptr->recv_buff_info.total_proc_len=0;
  }

  if(*close_flag)//连接需要被关闭
  {
     //epoll_del(fd_epoll, ptr->fd);
     //close(ptr->fd);//需要先删除监听,再关闭socket;故不能在此处close();
     free_conn_info(ptr);
  }
  return 0;
}

//发送数据
int do_write_data(int so, bool *close_flag)
{
   if(NULL!=close_flag) *close_flag=false;

   if(so<=0) //无效文件描述符
      return -1;

   //找到socket对应的连接
   client_t *ptr=get_conn_info(so);
   if(NULL==ptr)
      return -2;

   //没有数据发送
   if(ptr->snd_buff_info.data_len<=0)
   {
   }
   else if(ptr->snd_buff_info.data_len < ptr->snd_buff_info.total_proc_len)//数据发送完毕
   {
      //清理发送缓存
      memset(ptr->snd_buff_info.buff, 0, ptr->snd_buff_info.buff_size);
      ptr->snd_buff_info.data_len=0;
      ptr->snd_buff_info.total_proc_len=0;
   }

   //获取下一个发送数据
   if(ptr->snd_buff_info.data_len<=0)
   {
   }

   //找到下一个发送数据
   if(ptr->snd_buff_info.data_len>0)
   {
     //发送数据
     char *snd_pos=ptr->snd_buff_info.buff + ptr->snd_buff_info.total_proc_len;
     int snd_size=ptr->snd_buff_info.data_len - ptr->snd_buff_info.total_proc_len;
     send(ptr->fd, snd_pos, snd_size, 0);
     return snd_size;
   }

   //不存在下一个发送数据

   return 0;
}

//功能: 接收新连接
//返回值: >0-新连接描述符; <=0-失败;
int do_accept_client() 
{
  struct epoll_event ev;

  struct sockaddr_in cliaddr;//客户端地址;
  socklen_t cliaddr_len = sizeof(cliaddr);

  int conn_fd = accept(fd_listen, (struct sockaddr *)&cliaddr, &cliaddr_len);
  if (conn_fd < 0) {
    ERROR_MSG("accept client link error! [%d][%s]", errno, strerror(errno));
    return -1;
  }
  //if (lxx_net_set_socket(conn_fd, false) != 0) {
  //    close(conn_fd);
  //    fprintf(stderr, "do_accept_client failed(setnonblock)[%s:%d]\n",
  //    inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
  //    return;
  //  }

  int i = 0;
  bool flag = true;

  // 寻找合适的连接资源
  client_t *ptr=NULL;
  ptr=alloc_conn_info();
  if(NULL==ptr)//无可用连接
  {
     //此时连接尚未增加到监听,直接关闭即可;
     close(conn_fd);//注意:需要关闭连接
     ERROR_MSG("do_accept_client failed(not found unuse client)[%s:%d]",
                inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
     return -2;
  }
    ptr->port = cliaddr.sin_port;
    snprintf(ptr_cli[i].host, sizeof(ptr_cli[i].host), inet_ntoa(cliaddr.sin_addr));
    ptr->len = 0;
    ptr->fd = conn_fd;
    ptr->status = true;

    //设置缓存信息
    ptr->recv_buff_info.buff = ptr->recv_buff;
    ptr->recv_buff_info.buff_size = sizeof(ptr->recv_buff)-1;
    ptr->recv_buff_info.data_len=0;
    ptr->recv_buff_info.total_proc_len=0;
    ptr->snd_buff_info.buff=ptr->snd_buff;
    ptr->snd_buff_info.buff_size = sizeof(ptr->snd_buff);
    ptr->snd_buff_info.data_len=0;
    ptr->snd_buff_info.total_proc_len=0;
    //flag=false;
  

  //添加到监听队列
  //ev.events = EPOLLIN; //只监听接收数据事件,不监听发送事件
  //ev.data.u32 = i | 0x10000000;//保存连接索引(自定义)
  //if (epoll_add(fd_epoll, conn_fd, &ev) < 0) {
  //      ptr_cli[i].status = false;
  //      close(conn_fd);
  //      ERROR_MSG("do_accept_client failed(epoll_add)[%s:%d]",
  //         inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
  //      return -3;
  // }
  //
  // INFO_MSG("do_accept_client success[%s:%d]",
  //          inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
  
  return ptr->fd;
}

//功能:连接被关闭处理事件
int do_disconn(int so, bool *close_flag)
{
   if(NULL!=close_flag) *close_flag=false;

   client_t *ptr=get_conn_info(so);
   if(NULL==ptr)
      return -2;

   //close(ptr->fd);//需要先删除监听,再关闭socket;故不能在此处close();
   free_conn_info(ptr);
   if(NULL!=close_flag) 
      *close_flag=true;
   return 0;
}

int main(int argc, char **argv) 
{
  int rc;

  //socket信息
  unsigned short port = 12345;
  unsigned int so_concurrency=100;//连接并发数

  //日志信息 
  char log_file_path[256]={"log/"};//日志文件路径
  char log_file_prefix[64]={0};//日志文件名前缀
  
  //epoll配置信息
  int epoll_max_size=MAX_EPOLL_SIZE; //epoll句柄监听最大文件数
  int epoll_wait_timeout=5000;//epoll_wait超时时间,单位毫秒; 0-立即返回; -1不确定;

  //解析命令行参数
  if(argc == 2){
      port = atoi(argv[1]);
  }

  //读取ini配置文件
  //

  //初始化日志
  CLogFileManage *logfile=log_file_init(log_file_path, NULL);
  if(NULL==logfile)
  {
    printf("FATAL: 初始化日志文件信息失败!\n");
    return -3;
  }
  
  //启动监听端口
  //if ((fd_listen = lxx_net_listen(port, QUEUE_LEN)) < 0) {
  //  fprintf(stderr, "listen port failed[%d]", port);
  //  return -1;
  //}
  if((fd_listen=socket(AF_INET, SOCK_STREAM, 0))<0)
  {
     FATAL_MSG("create socket  for server port error!");
     return -4;
  }

  printf("success to create socket server!\n");

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family=AF_INET;
  server_addr.sin_port=htons(port);
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(fd_listen, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
  {
     close(fd_listen);
     fd_listen=0;
     FATAL_MSG("bind port[%d] failed!", port);
     return -5;
  }

  printf("success to bind server port[%d]!\n", port);

  if(listen(fd_listen, MAX_CLIENT_SIZE)<0)
  {
     close(fd_listen);
     fd_listen=0;
     FATAL_MSG("listen(%d) failed!", port);
     return -6;
  }

  printf("sucess to listen server port!\n");

  /*NOTE_WARN_0000: 连接对方崩溃,则epoll没法检测出来,需要在业务层想法使用心跳*/
  //创建epoll句柄
  fd_epoll = epoll_create(epoll_max_size);
  if (fd_epoll < 0) {
    close(fd_listen);
    fd_listen=0;
    FATAL_MSG("create epoll failed.%d\n", fd_epoll);
    return -1;
  }

  printf("success to create epoll handle!\n");

  // 将监听连接加入事件集合
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
  ev.data.fd = fd_listen;
  if (epoll_add(fd_epoll, fd_listen, &ev) < 0) {
    close(fd_epoll);
    close(fd_listen);
    fd_epoll = -1;
    fd_listen = -1;
    FATAL_MSG("server port add to epoll wait pool failed!");
    return -1;
  }

  printf("success to add server socket to epoll monitor!\n");

  ptr_cli = new client_t[MAX_CLIENT_SIZE];//客户端连接

  struct epoll_event events[MAX_EPOLL_SIZE];//监听最大事件数量
  int epoll_sig_interrpt_count=0;//epoll被signal中断次数
  bool conn_close_flag=false;//连接断开标记
  for (;;) {
   
    TEST_MSG("start to epoll_wait...\n");
    int nfds = epoll_wait(fd_epoll, events, MAX_EPOLL_SIZE, epoll_wait_timeout);
    TEST_MSG("epoll_wait return fd count[%d]... \n\n", nfds);

    if (nfds < 0) {
      int err = errno;
      if (err != EINTR) {
        FATAL_MSG("epoll_wait failed[%s]", strerror(err));
      }
      
      ++epoll_sig_interrpt_count;
      if(100==(epoll_sig_interrpt_count%101))
      {
        DEBUG_MSG("epoll_wait has been interrupted by signal [%d] times",epoll_sig_interrpt_count);
        if(epoll_sig_interrpt_count>=10000) 
           epoll_sig_interrpt_count=0;
      }
      continue;
    }//没有有效数据通知


    //逐个处理每个socket连接事件
    for (int i = 0; i < nfds; i++) {
      memset(&ev, 0, sizeof(ev));
      if(events[i].data.fd == fd_listen)//服务断开收到新的连接请求
      {
        /*NOTE_WARN_0005  是否考虑服务端口socket出错的情况;太复杂了,还是算了;*/
        //接收新连接
        rc=do_accept_client();
          
        /*添加一次即可,不需要重复添加*/
        ////服务端口:重新添加到epoll监听队列
        //ev.events = EPOLLIN | EPOLLET; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
        //ev.data.fd = fd_listen;
        //if (epoll_add(fd_epoll, fd_listen, &ev) < 0)
        //{
        //    FATAL_MSG("server端口重新添加到epoll队列失败!不会再接收新连接请求");
        //}

        //新连接: 添加到epoll监听队列
        //
        if(rc>0)
        {
          ev.events = EPOLLIN |EPOLLHUP| EPOLLET; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
          ev.data.fd = rc;
          if (epoll_add(fd_epoll, rc, &ev) < 0)
          {
            /* 注意: NOTE_WARN_0002 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除, 但由于连接刚建立,接收缓存没有数据,  但发送缓存可能存在数据;*/
            conn_close_flag=false;
            do_disconn(rc,&conn_close_flag);//关闭连接
            close(rc);
            FATAL_MSG("新连接添加到epoll队列失败!无法接收新连接数据");
          }
        }//新连接添加epoll处理完成
      }//end of 服务端新连接完成
      else if(events[i].events & EPOLLIN)//读事件被触发(断开连接也会触发该事件)
      {
         //连接断开还是有新数据读取??
         /* NOTE_WARN_002: 挺奇怪的 测试发现: epoll竟然没有捕获EPOLLHUP事件???*/
         if(events[i].events & EPOLLHUP)//断开连接,epoll触发EPOLLIN + EPOLLHUP
         {
            /* 注意: WARN_0001 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除,接收缓存可能存在待处理数据*/
            //close(events[i].data.fd);
            conn_close_flag=false;
            rc=do_disconn(events[i].data.fd, &conn_close_flag);
            epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
            continue;
         }
 
        conn_close_flag=false;
        rc=do_read_data(events[i].data.fd, &conn_close_flag);
        if(!conn_close_flag)//连接未断开,需要重新添加到监听队列
        {
          /*添加一次即可,不需要重复添加*/
          //ev.events = EPOLLIN | EPOLLHUP | EPOLLET ; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
          //ev.data.fd = fd_listen;
          //if (epoll_add(fd_epoll, events[i].data.fd, &ev) < 0)
          //{
          //   FATAL_MSG("client 连接重新注册[read]事件失败,该连接不会再接收数据!");

          //   TEST_MSG("read 重新添加监听失败!");

          //   conn_close_flag=true;//添加监听失败,则断开连接
          //   /* 注意: NOTE_WARN_0001 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除,接收缓存可能存在待处理数据*/
          //   rc=do_disconn(events[i].data.fd, &conn_close_flag);
          //   epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
          //}
        }
        if(conn_close_flag)//需要断开连接
        {
          /* 注意: NOTE_WARN_0001 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除,接收缓存可能存在待处理数据*/
          rc=do_disconn(events[i].data.fd, &conn_close_flag);
          epoll_del(fd_epoll,events[i].data.fd);//删除监听
        }
         
      }//读取连接完成
      else if(events[i].events & EPOLLOUT) //可写数据
      {
         conn_close_flag=false;
         
         rc=do_write_data(events[i].data.fd, &conn_close_flag);//发起下一次写操作
         
        //连接未断开,则重新添加到监听队列
        if(!conn_close_flag)
        {
          ev.events = EPOLLOUT | EPOLLET ; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
          ev.data.fd = fd_listen;
          if (epoll_add(fd_epoll, events[i].data.fd, &ev) < 0)
          {
             FATAL_MSG("client connection 连接重新注册[write]事件失败,该连接不会写接收数据!");

             conn_close_flag=true;

             /* 注意: NOTE_WARN_0001 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除,接收缓存可能存在待处理数据*/
             ////rc=do_disconn(events[i].data.fd, &conn_close_flag);
             //epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
             //rc=do_disconn(events[i].data.fd, &conn_close_flag);
          }
          if(conn_close_flag)
          {
             /* 注意: NOTE_WARN_0001 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除,接收缓存可能存在待处理数据*/
             ////rc=do_disconn(events[i].data.fd, &conn_close_flag);
             //epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
             rc=do_disconn(events[i].data.fd, &conn_close_flag);
             epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
          }
        }//end 重新添加到监听队列
        else
        {  
           char ip_tmp[64]={0};
           int port_tmp=0;
           struct sockaddr_in addr_tmp;
           socklen_t addr_tmp_len=sizeof(addr_tmp);
           memset(&addr_tmp, 0, sizeof(addr_tmp));
           getsockname(events[i].data.fd, (struct sockaddr*)&addr_tmp, &addr_tmp_len);

           strcpy(ip_tmp,inet_ntoa(addr_tmp.sin_addr));
           port_tmp = addr_tmp.sin_port;
           if(events[i].events & EPOLLHUP)
           {
              TEST_MSG("\n   Error: [%s:%d] EPOLLHUP \n", ip_tmp, port_tmp);
           }
           if(events[i].events & EPOLLRDHUP)//部分系统检测不到该事件,read代替(read返回0,则删除事件,close(fd)); 如果能检测到,则使用该事件;
           {
              TEST_MSG("\n   Error: [%s:%d] EPOLLRDHUP \n", ip_tmp, port_tmp);
           }
           if(events[i].events & EPOLLPRI)
           {
              TEST_MSG("\n   Error: [%s:%d] EPOLLPRI \n", ip_tmp, port_tmp);
           }
           if(events[i].events & EPOLLERR)
           {
              TEST_MSG("\n   Error: [%s:%d] EPOLLERR \n", ip_tmp, port_tmp);
           }
           
           /* 注意: NOTE_WARN_0001 此处直接调用do_disconn会将该连接的接收缓存、发送缓存一块清除,接收缓存可能存在待处理数据*/
           if(events[i].data.fd==fd_listen)
           {
             /* NOTE_WARN_OO03 是否应该删除服务端口的监听;*/
             /* NOTE_WARN_0004 考虑到服务端口有问题,但是连接还正常,仍然需要继续工作*/
             epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
           }
           else
           {
             rc=do_disconn(events[i].data.fd, &conn_close_flag);
             epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
           }
        }
      }//写数据完成

    }//下一个连接处理
  }//继续等待



  //禁用监听
  epoll_del(fd_epoll, fd_listen);//删除连接监听
  //for任何一个连接, 删除监听
  if(0!=fd_epoll)//关闭epoll文件
  {
    close(fd_epoll);
    fd_epoll=0;
  }

  //清除连接
  close(fd_listen);
  fd_listen=0;
  //for任何一个连接, 关闭连接

  //释放内存
  if(NULL!=ptr_cli)
  { 
     delete [] ptr_cli;
     ptr_cli=NULL;
  }

  //关闭各个线程

  //关闭日志
  return 0;
}
