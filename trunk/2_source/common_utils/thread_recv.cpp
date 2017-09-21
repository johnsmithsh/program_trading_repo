/*
 * myepoll.cpp
 *
 * Created on: 2013-06-03
 * Author: liuxiaoxian
 * 提高ms并发度调研：把客户端发来的数据发过去
 */

#include "log.h" //日志


#include "mxx_net_socket.h"
#include "mxx_socket_event.h"
#include "thread_recv.h"
#include "itc_mutex.h"
#include "SocketConnInfo.h"
#include "SocketConnPool.h"
#include "os_time.h"
#include "mxx_epoll.h"


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

#define MAX_EPOLL_SIZE 500 //epoll句柄监听最大连接数

#define MAX_CLIENT_SIZE 500
#define MAX_IP_LEN      16
#define MAX_CLIENT_BUFF_LEN 1024
#define QUEUE_LEN 500
#define BUFF_LEN 1024
int fd_epoll = -1;
int fd_listen = -1;

#define TEST_MSG printf

/////////////////////////////////////////////////////////////////////////////////////
//定义接收缓存
typedef struct _recv_pack
{
  ItcMutex mutex;//锁
  std::deque<ST_BIN_BUFF *> recv_que;
} ST_RECV_QUE;

ST_RECV_QUE g_recv_que;

ST_RECV_QUE * get_recv_que()
{
    return &g_recv_que;
}

int recv_que_push(ST_BIN_BUFF *bin_buff)
{
   if(NULL==bin_buff) return 0;

   ST_RECV_QUE *que=get_recv_que();
   if(NULL==que) return -1;

   que->mutex.lock();
   que->recv_que.push_back(bin_buff);
   que->mutex.unlock();
   
}

ST_BIN_BUFF * recv_que_pop()
{
   ST_RECV_QUE *que=get_recv_que();
   if(NULL==que) return NULL;

   ST_BIN_BUFF *bin_buff;
   que->mutex.lock();
   bin_buff=que->recv_que.front();
   que->recv_que.pop_front();
   que->mutex.unlock();
   return bin_buff;
}

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
//socket连接对象
//
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

////////////////////////////////////////////////////////////////////////
//构造函数
CRecvThread::CRecvThread(char *thread_name/*="recv_thread"*/):Thread_Base(thread_name)
{
   m_server_port=0;
}

int CRecvThread::init()
{
   return 0;
}

int CRecvThread::terminate()
{
   return 0;
}


// 接收数据
int do_read_data(int so, bool *close_flag) {
  if(NULL!=close_flag) *close_flag=false;

  if (so<=0) {//无效文件描述符
    return -1;
  }
  //int n=0;
  int rc;

  //查找socket表示的网络连接;
  CSocketConnPool *recv_conn_pool=mxx_get_socket_conn_pool(ConnPool_RCV);//连接池
  ST_SocketConnInfo *conn_ptr=mxx_get_socket_conn_info(ConnPool_RCV,so);
  if(NULL==conn_ptr)
  {
     ERROR_MSG("无法根据socket描述符找到对应的连接信息!");
     return -2;
  }

  //int count=0;
  //int sum_count=0;
  //bool is_more_data=true;//是否有更多的数据需要读取; true-有; false-没有;
  ST_BIN_DATA_HEAD bin_head;//报文头
  ST_BIN_BUFF *bin_buff=NULL;
  
  //socket连接缓存
  ST_SOCKET_BUFF_INFO sock_buff;
  memset(&sock_buff, 0, sizeof(sock_buff));

  //开始读取...
  for(;;)
  {
     memset(&sock_buff, 0, sizeof(sock_buff));

     //判断是否存在下一个协议包
     rc=recv(so, &bin_head, sizeof(bin_head), MSG_PEEK|MSG_DONTWAIT);
     if(rc<0)
     {
        if((EAGAIN==rc) || (EWOULDBLOCK==rc))//没有数据
           break;
        else if(EINTR==rc)//被中断,下次读取吧!
           break;
        else //其他错误
        {
           if(NULL!=close_flag) *close_flag=true;
           break;
        }
     }
     else if(0==rc)//socket连接关闭
     {
        if(NULL!=close_flag) *close_flag=true;
        
        break;
     }
     else if(rc<sizeof(bin_head))//缓存长度不足一个报文头
        break;


     //bin_buff=NULL;//上一次读取的报文已进入队列,不跟踪不会造成内存泄漏;
     rc=mxx_read_event(so, &sock_buff);//一次读取一个协议包
     bin_buff=sock_buff.buff;
     if(0!=rc)//读取失败
     {
       if(MXX_ERR_BIN_PACK_HEAD_UNCOMPLETE==rc)//协议报文头都没有读取完整
       {
          //该连接缓存中剩下的缓存怎么处理???
       }
       else if(MXX_ERR_BIN_PACK_UNCOMPLETE==rc)//协议报文不完整
       {
          //是否下次继续读取???...
       }
       else if( (MXX_ERR_BIN_PACK_HEAD_TOSMALL==rc) || (MXX_ERR_BIN_PACK_HEAD_TOLONG==rc) )//协议包太长或太短
       {
       }
       else if(MXX_ERR_CONN_CLOSE==rc)//对方关闭连接
       {
          //判断当前报文是否完整,如果完整则继续
          //

          if(NULL!=close_flag) *close_flag=true;
       }
       else if(MXX_ERR_BIN_PACK_ALLOC_FAILED==rc)//申请缓存失败
       {
       }
       if(NULL!=sock_buff.buff)
         mxx_free_bin_pack(sock_buff.buff);
       break;
     }

     //校验完整性
     bool is_intergrality=true;
     if(!is_intergrality)
     {
        if(NULL!=sock_buff.buff)
        {
          mxx_free_bin_pack(sock_buff.buff);
          break;
        }
     }

     //获取缓存队列...
     recv_que_push(sock_buff.buff);

     //读取下一个数据包
  }
  
  if(*close_flag)//连接需要被关闭
  {
     //epoll_del(fd_epoll, ptr->fd);
     //close(ptr->fd);//需要先删除监听,再关闭socket;故不能在此处close();
     //free_conn_info(conn_ptr);
     recv_conn_pool->remove_socketConnInfo(so);
  }
  return 0;
}

//发送数据
int do_write_data(int so, bool *close_flag)
{
   if(NULL!=close_flag) *close_flag=false;

   if(so<=0) //无效文件描述符
      return -1;

   for(;;)
   {
     //下一个发送缓存...
     ST_SOCKET_BUFF_INFO *snd_bin_buff=NULL;
     if(NULL==snd_bin_buff)
        break;

     //找到socket对应的连接
     ST_SocketConnInfo *conn_ptr=mxx_get_socket_conn_info(ConnPool_SND,so);
     if(NULL==conn_ptr)
     {
        if(NULL!=snd_bin_buff->buff)
        {
          mxx_free_bin_pack(snd_bin_buff->buff);
          snd_bin_buff->buff=NULL;
        }
        continue;
     }

   
     //开始发送...
     int rc=mxx_write_event(so, snd_bin_buff);
     if(rc<0)
     {
       if(MXX_ERR_CONN_CLOSE==rc)//socket关闭
       {
       }
       else if(MXX_ERR_TIMEOUT==rc)//超时
       {
       }
       else //其他错误
       {
       }
     }
     else if(rc<snd_bin_buff->buff->cur_data_len+sizeof(ST_BIN_DATA_HEAD))//没有发送完
     {
     }
   }

   return 0;
}

//功能: 接收新连接
//返回值: >0-新连接描述符; <=0-失败;
int do_accept_client() 
{
  //struct epoll_event ev;

  struct sockaddr_in cliaddr;//客户端地址;
  socklen_t cliaddr_len = sizeof(cliaddr);

  int rc;

  int conn_fd = accept(fd_listen, (struct sockaddr *)&cliaddr, &cliaddr_len);
  if (conn_fd < 0) {
    ERROR_MSG("accept client link error! [%d][%s]", errno, strerror(errno));
    return -1;
  }

  //创建conn连接信息
  ST_SocketConnInfo conn_info;
  memset(&conn_info, 0, sizeof(conn_info));


  ST_SocketConnInfo *conn_ptr=&conn_info; //conn_ptr=alloc_conn_info();
  if(NULL==conn_ptr)//无可用连接
  {
     //此时连接尚未增加到监听,直接关闭即可;
     close(conn_fd);//注意:需要关闭连接
     ERROR_MSG("do_accept_client failed(not found unuse client)[%s:%d]",
                inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
     return -2;
  }

  conn_ptr->sci_id=get_conn_id(conn_fd);//暂时使用文件描述符做连接id吧;
  conn_ptr->sci_conn_tppe=SCI_CONN_TYPE_TCP;
  conn_ptr->sci_recv_last_net_serial_no=0;//最近接收序号
  conn_ptr->sci_snd_last_net_serial_no=0;//最近的发送序号
  conn_ptr->sci_conn_timeout=0;//超时间设置为0,即没有超时时间
  
  conn_ptr->sci_sock_fd = conn_fd;//文件描述符
  conn_ptr->sci_cs_flag=SCI_CS_FLAG_SERVER;//作为服务器
  
  //获取socket本地ip和port、远程ip和port...
  struct sockaddr_in addr;
  socklen_t addr_len=sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  //获取对方ip和端口
  if((rc=getpeername(conn_fd, (struct sockaddr *)&addr, &addr_len))<0)
  {
     close(conn_fd);//注意:需要关闭连接
     ERROR_MSG("do_accept_client: getpeername() failed, rc=[%d], errno=[%d], strerr=[%s]", rc, errno, strerror(errno));
     return -2;
  }
  inet_ntop(addr.sin_family, &addr.sin_addr, (char*)conn_ptr->sci_remote_ip, sizeof(conn_ptr->sci_remote_ip));//对方ip
  conn_ptr->sci_remote_port = ntohs(addr.sin_port);
  
  addr_len=sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  if((rc=getsockname(conn_fd, (struct sockaddr *)&addr, &addr_len))<0)//获取本地端口和ip
  {
     close(conn_fd);//注意:需要关闭连接
     ERROR_MSG("do_accept_client: getsockname() failed, rc=[%d], errno=[%d], strerr=[%s]", rc, errno, strerror(errno));
     return -2;
  }
  inet_ntop(addr.sin_family, &addr.sin_addr, (char *)conn_ptr->sci_local_ip, sizeof(conn_ptr->sci_local_ip));//对方ip
  conn_ptr->sci_local_port = ntohs(addr.sin_port);

  //初始化时间...
  os_get_timeval(&conn_ptr->sci_start_time);//设置连接建立时间
  conn_ptr->sci_last_recv_time = conn_ptr->sci_start_time;//上次收到协议包时间
  conn_ptr->sci_last_snd_time = conn_ptr->sci_start_time;//上次发送协议包时间

  //设置统计信息...
  ST_SocketConnStatics *conn_static_ptr=&conn_ptr->sci_conn_statics;//获取统计信息指针
  conn_static_ptr->scs_recv_netpack_count=conn_static_ptr->scs_snd_netpack_count=0;//协议包接收计数与协议包发送计数;
  conn_static_ptr->scs_avg_load=0;//队列平均负载;
  conn_static_ptr->scs_max_load=0;//队列最大负载;
  conn_static_ptr->scs_cur_laod=0;//队列当前负载;
  conn_static_ptr->scs_support_max_load=0;//支持的最大负载

  //协议包处理时间,统计协议包request/response的耗时
  conn_static_ptr->scs_max_cost_time=0;//最大耗时时间,单位:毫秒
  conn_static_ptr->scs_avg_cost_time=0;//平均耗时时间,单位:毫秒
  conn_static_ptr->scs_cur_cost_time=0;//最近若干协议包处理耗时时间,单位:毫秒

  //添加到连接池
  CSocketConnPool *conn_pool_ptr=mxx_get_socket_conn_pool(ConnPool_RCV);
  conn_pool_ptr->set_socketConnInfo(conn_ptr->sci_id, conn_ptr);

  //在该level socket发送必须串行,发送同样如此
  //发送与接收可以并行,故需要发送缓存与接收缓存两个;
  //    ST_SOCKET_BUFF_INFO recv_buff_info;//接收缓存info
  //    ST_SOCKET_BUFF_INFO snd_buff_info; //发送缓存info
  //ptr->len = 0;
  //ptr->fd = conn_fd;
  //ptr->status = true;

  //设置缓存信息,这是为了防止一次epoll循环中只能读取部分数据,下次需要接着读取
  //ptr->recv_buff_info.buff = ptr->recv_buff;
  //ptr->recv_buff_info.buff_size = sizeof(ptr->recv_buff)-1;
  //ptr->recv_buff_info.data_len=0;
  //ptr->recv_buff_info.total_proc_len=0;
  //ptr->snd_buff_info.buff=ptr->snd_buff;
  //ptr->snd_buff_info.buff_size = sizeof(ptr->snd_buff);
  //ptr->snd_buff_info.data_len=0;
  //ptr->snd_buff_info.total_proc_len=0;
  

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
  
  return conn_ptr->sci_sock_fd;
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

int CRecvThread::recv_routine() 
{
  int rc;

  //socket信息
  unsigned short m_lstn_port = 12345;
  unsigned int so_concurrency=100;//连接并发数
  
  //epoll配置信息
  int epoll_max_size=MAX_EPOLL_SIZE; //epoll句柄监听最大文件数
  int epoll_wait_timeout=5000;//epoll_wait超时时间,单位毫秒; 0-立即返回; -1不确定;

  
  
  //启动监听端口
  if((fd_listen=socket(AF_INET, SOCK_STREAM, 0))<0)
  {
     FATAL_MSG("thread_recv: create socket  for server port error!");
     return -4;
  }

  printf("thread_recv: success to create socket server!\n");

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family=AF_INET;
  server_addr.sin_port=htons(m_lstn_port);
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(fd_listen, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
  {
     close(fd_listen);
     fd_listen=0;
     FATAL_MSG("thread_recv: bind port[%d] failed!", m_lstn_port);
     return -5;
  }

  printf("thread_recv: success to bind server port[%d]!\n", m_lstn_port);

  if(listen(fd_listen, MAX_CLIENT_SIZE)<0)
  {
     close(fd_listen);
     fd_listen=0;
     FATAL_MSG("thrread_recv: listen(%d) failed!", m_lstn_port);
     return -6;
  }

  printf("thread_recv: sucess to listen server port!\n");

  /*NOTE_WARN_0000: 连接对方崩溃,则epoll没法检测出来,需要在业务层想法使用心跳*/
  //创建epoll句柄
  fd_epoll = epoll_create(epoll_max_size);
  if (fd_epoll < 0) {
    close(fd_listen);
    fd_listen=0;
    FATAL_MSG("thread_recv: create epoll failed.%d\n", fd_epoll);
    return -1;
  }

  printf("thread_recv: success to create epoll handle!\n");

  // 将监听连接加入事件集合
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
  ev.data.fd = fd_listen;
  if (mxx_epoll_add(fd_epoll, fd_listen, &ev) < 0) {
    close(fd_epoll);
    close(fd_listen);
    fd_epoll = -1;
    fd_listen = -1;
    FATAL_MSG("thred_recv: server port add to epoll wait pool failed!");
    return -1;
  }

  printf("thread_recv: success to add server socket to epoll monitor!\n");


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
          if (mxx_epoll_add(fd_epoll, rc, &ev) < 0)
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
            mxx_epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
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
          mxx_epoll_del(fd_epoll,events[i].data.fd);//删除监听
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
          if (mxx_epoll_add(fd_epoll, events[i].data.fd, &ev) < 0)
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
             mxx_epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
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
             mxx_epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
           }
           else
           {
             rc=do_disconn(events[i].data.fd, &conn_close_flag);
             mxx_epoll_del(fd_epoll, events[i].data.fd);//删除连接监听
           }
        }
      }//写数据完成

    }//下一个连接处理
  }//继续等待



  //禁用监听
  mxx_epoll_del(fd_epoll, fd_listen);//删除连接监听
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
