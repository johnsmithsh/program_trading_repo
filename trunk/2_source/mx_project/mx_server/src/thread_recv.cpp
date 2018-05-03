/*
 * 接收线程
 *
 * Created on: 
 * Author: 
 * 
 */

#include "logfile.h" //日志


#include "mxx_net_socket.h"
//#include "mxx_socket_event.h"
#include "thread_recv.h"
#include "itc_mutex.h"
#include "SocketConnInfo.h"
//#include "SocketConnPool.h"
#include "socket_handle.h" 
#include "os_time.h"
#include "mxx_epoll.h"
#include "sock_frame.h"
#include "ConfigFile.h"

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
//#define MAX_IP_LEN      16
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
  std::deque<ST_SOCK_FRAME *> recv_que;
} ST_RECV_QUE;

ST_RECV_QUE g_recv_que;

ST_RECV_QUE * get_recv_que()
{
    return &g_recv_que;
}

int recv_que_push(ST_SOCK_FRAME *bin_buff)
{
   if(NULL==bin_buff) return 0;

   ST_RECV_QUE *que=get_recv_que();
   if(NULL==que) return -1;

   //ST_BIN_BUFF *bin_buff=NULL;
   que->mutex.lock();
   que->recv_que.push_back(bin_buff);
   //bin_buff=que->recv_que.front();
   que->mutex.unlock();
   
}

ST_SOCK_FRAME*  recv_que_pop()
{
   ST_RECV_QUE *que=get_recv_que();
   if(NULL==que) return NULL;

   ST_SOCK_FRAME *bin_buff=NULL;
   int size=0;
   que->mutex.lock();
   size=que->recv_que.size();
   if(size>0)
    {
     bin_buff=que->recv_que.front();
     que->recv_que.pop_front();
    }
   que->mutex.unlock();
   return bin_buff;
}

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
//socket连接对象
//


ST_SockConnHandle *g_client_ptr = NULL; //客户端连接

//功能:申请一个空闲网络连接
ST_SockConnHandle *alloc_conn_info()
{
   int i=0;
   ST_SockConnHandle *ptr=NULL;
   for(i=0; i<MAX_CLIENT_SIZE; i++)
   {
     ptr=g_client_ptr+i;
     if(ptr->status)
        continue;
     
     memset(ptr, 0, sizeof(ST_SockConnHandle));
     ptr->status=true;
     return ptr;
   }

   return NULL;
}

//功能:释放网络连接
void free_conn_info(ST_SockConnHandle *ptr)
{
   if(NULL==ptr) return;
   memset(ptr, 0, sizeof(ST_SockConnHandle));
   ptr->status=false;
   return;
}

//功能: 根据socket文件描述符找到对应的连接信息;
ST_SockConnHandle *get_conn_info(int so)
{
   int i=0;
   ST_SockConnHandle *ptr=NULL;
   for(i=0; i<MAX_CLIENT_SIZE; i++)
   {
     ptr=g_client_ptr+i;
     if( (ptr->fd==so) && (ptr->status) )
        return ptr;
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////
//构造函数
CRecvThread::CRecvThread(const char *thread_name/*="recv_thread"*/)
    :Thread_Base(thread_name),m_lstn_port(0), m_max_listen(0),m_recv_timeout(0),m_send_timeout(0),m_b_running(false),m_stop_flag(false)
{
   m_lstn_port=0;
   m_max_listen=0;
   m_max_listen=0;
   m_recv_timeout=m_send_timeout=0;
   m_b_running=false;
   m_stop_flag=false;
}
//析构函数
CRecvThread::~CRecvThread()
{
}

int CRecvThread::init()
{
   return 0;
}

int CRecvThread::terminate_service()
{
    m_stop_flag=true;
    return 0;
}

int CRecvThread::loadini(char *cfgfile)
{
    ConfigFile cfg;
    int rc=cfg.load_cfg_file(cfgfile);
    if(rc < 0)
    {
        ERROR_MSG("打开配置文件失败, rc=[%d]", rc);
        return -1;
    }
    
    char serve_section[]="SERVER";
    m_lstn_port   =cfg.read_int(serve_section,  "serverport", 9999);//!< 服务监听端口
    m_max_listen  =cfg.read_int(serve_section,  "max_conn",   1024); //!< 最大连接数
    m_recv_timeout=cfg.read_int(serve_section,  "timeout",    1000); //!< 接收超时,单位毫秒
    m_send_timeout = m_recv_timeout; //!< 发送超时时间,单位毫秒
    
    if(m_lstn_port<=0)
    {
        FATAL_MSG("[%s][%s]服务端口配置不合法!", serve_section, "serverport");
        return -1;
    }
    if(m_max_listen<1)
    {
        FATAL_MSG("[%s][%s]最大连接数配置不合法!", serve_section, "max_conn");
        return -1;
    }
    
    if( ((m_recv_timeout<0)||(m_recv_timeout>3000))
        ||((m_send_timeout<0)||(m_send_timeout>3000))
      )
    {
        FATAL_MSG("[%s][%s]超时时间配置不合法!范围[0,3000]", serve_section, "timeout");
        return -1;
    }
    
    return 0;   
}

// 接收数据
// 参数:
//   [in]so: socket fd表示一个socket连接;
//   [out]close_flag: 如果在读取过程中,发现连接被关闭或出现异常,则设置该标记
//         注:该函数不会关闭so连接;
// 返回值:
//    <0-失败; 0-成功;
int do_read_data(int so, bool *close_flag) {
  if(NULL!=close_flag) *close_flag=false;

  if (so<=0) {//无效文件描述符
    return -1;
  }
  //int n=0;
  int rc;

  //获取连接句柄
  ST_SockConnHandle *client_ptr=get_conn_info(so);
  if(NULL==client_ptr)
  {
    ERROR_MSG("Error: do_read_data failed! 找不到连接句柄");
    return -2;
  }
  //查找socket表示的网络连接;
  //CSocketConnPool *recv_conn_pool=mxx_get_socket_conn_pool(ConnPool_RCV);//连接池
  //ST_SocketConnInfo *conn_ptr=mxx_get_socket_conn_info(ConnPool_RCV,so);
  ST_SocketConnInfo *conn_ptr=&client_ptr->conn_info;
  if(NULL==conn_ptr)
  {
     ERROR_MSG("无法根据socket描述符找到对应的连接信息!");
     return -2;
  }
  
  //socket连接缓存
  ST_SOCK_FRAME_HEAD bin_head;
  char data_buff[8192];//!< 接收数据缓存;
  int data_len;//!< 接收数据长度
  //memset(&data_buff, 0, sizeof(data_buff));//!< 由于data_buff比较大,故不适用memset,防止影响性能

  //ST_SOCK_FRAME *socket_frame=NULL;
  
  //开始读取(每个for循环读取一个协议包)...
  //int bin_pack_count=0;
  for(;;)
  {
      //判断是否存在下一个协议包: peek报文头
	  //peek报文头
	  memset(&bin_head, 0, sizeof(ST_SOCK_FRAME_HEAD));
	  rc=recv(so, &bin_head, sizeof(ST_SOCK_FRAME_HEAD), MSG_PEEK|MSG_DONTWAIT);
	  if(rc<0)
	  {
          if((EAGAIN==errno) || (EWOULDBLOCK==errno))//没有数据
              break;
		  else if(EINTR==errno)//被中断,下次读取吧
		      break;
		  else //其他错误
          {
		      ERROR_MSG("peek报文头失败!未知错误,errno=[%d], strerr=[%s]", errno, strerror(errno));
              if(NULL!=close_flag) 
                  *close_flag=true;
              break;
          }
     }
     else if(0==rc)//socket在读取过程中连接关闭
     {
        if(NULL!=close_flag) *close_flag=true;
        
        break;
     }
     
	 //校验报文头
	 bool b_head=false;
	 int  pkg_len=0;//!< 数据包长度
	 int  rcv_len=0;//!< 实际接收数据长度
	 if((unsigned int)rc<sizeof(ST_SOCK_FRAME_HEAD))//缓存长度不足一个报文头,可能属于前一个数据包
     {
        //不应该出现该情况
        ERROR_MSG("peek报文头,长度[%d]<sizeof(head)", rc);
		//todo 可能是上一个,判断上一个报文是否完整
		//     如果完整,则返回错误;
		//     如果不完整, 则将计算缺失数据长度,并读取数据并添加到上一个报文中;
        break;
     }
	 else if(!framehead_is_valid(&bin_head))//不是报文头,可能属于上一个数据包;
	 {
	    ERROR_MSG("数据不是报文头");
	    //todo 可能是上一个,判断上一个报文是否完整
		//     如果完整,则返回错误;
		//     如果不完整, 则将计算缺失数据长度,并读取数据并添加到上一个报文中;
		break;
	 }
     else//报文头
	 {
	     b_head=true;
		 
		 pkg_len=bin_head.frame_len;
	 }
	 
	 //校验报文长度
	 if(pkg_len<=0)
	 {
	     ERROR_MSG("数据包长度为[%d]", pkg_len);
		 break;	     
	 }
	 else if(pkg_len>MAX_FRAME_SIZE)
	 {
	     ERROR_MSG("报文过长[%d]", pkg_len);
		 break;
	 }

	 ST_SOCK_FRAME      frame_data;
	 if(b_head)//读取一个新报文
	 {
	     memset(&frame_data.frame_head, 0, sizeof(ST_SOCK_FRAME_HEAD));
	     rc=mxx_socket_recv_waitall(so, (char *)frame_data.frame_data, pkg_len, &rcv_len);
		 if(rc<0)
		 {
		     if(MXX_SOCKET_BUFF_ERROR==rc)//缓冲区错误
			 {
			 }
			 else if(MXX_SOCKET_TIMEOUT==rc)//超时
			 {
			 }
			 else if(MXX_SOCKET_CLOSED==rc)//socket关闭
			 {
			 }
		 }
		 if(rcv_len<=0)//没有读取到数据
		 {
		 }
		 else if(rcv_len < pkg_len)//读取一部分数据
		 {
		 }
		 else if(rcv_len > pkg_len)//不可能出现
		 {
		 }
		 else// rcv_len==pkg_len, 报文读取完整
		 {
		     //todo 构建数据结构,添加到接收数据队列...
		 }
	 }
	 else //读取上一个报文数据
	 {
	     rc=mxx_socket_recv_waitall(so, data_buff, pkg_len, &rcv_len);
		 if(rc<0)
		 {
		     if(MXX_SOCKET_BUFF_ERROR==rc)//缓冲区错误
			 {
			 }
			 else if(MXX_SOCKET_TIMEOUT==rc)//超时
			 {
			 }
			 else if(MXX_SOCKET_CLOSED==rc)//socket关闭
			 {
			 }
		 }
		 if(rcv_len<=0)//没有读取到数据
		 {
		 }
		 else// rcv_len==pkg_len, 
		 {
		     //todo 获取上一个报文缓存并添加数据...
		 }
	 }
	 
     if(MXX_SOCKET_CLOSED==rc)//socket关闭
	 {
	     if(NULL!=close_flag) 
		     *close_flag=true;
		 //todo 下次监听不能再监听该socket; socket关闭应该在数据包处理完成后关闭
		 //
         break;
     }

     //校验完整性
     bool is_intergrality=true;
     if(!is_intergrality)
     {
        //if(NULL!=sock_buff.buff)
        //{
        //  mxx_free_bin_pack(sock_buff.buff);
        //  break;
        //}
     }

     //添加到接收队列
     //recv_que_push(&frame_data);

     //读取下一个数据包
  }//end of for(;;)
  
  if(*close_flag)//连接需要被关闭
  {
     //epoll_del(fd_epoll, ptr->fd);
     //close(ptr->fd);//需要先删除监听,再关闭socket;故不能在此处close();
     //free_conn_info(client_ptr);
     
     //recv_conn_pool->remove_socketConnInfo(so);
  }
  return 0;
}

//发送数据
int do_write_data(int so, bool *close_flag)
{
   if(NULL!=close_flag) *close_flag=false;

   if(so<=0) //无效文件描述符
      return -1;

   /*
   ST_SockConnHandle *client_ptr=get_conn_info(so);
   if(NULL==client_ptr)
   {
      ERROR_MSG("Error: do_write_data failed! 找不到sock连接句柄!");
      return -2;
   }
   for(;;)
   {
     //下一个发送缓存...
     ST_SOCKET_BUFF_INFO *snd_bin_buff=NULL;
     if(NULL==snd_bin_buff)
        break;
   
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
     else if(rc<mxx_bin_pack_len(snd_bin_buff->buff))//没有发送完
     {
     }
   }
   */
   
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
  int ret_code=0;//返回码

  int conn_fd = accept(fd_listen, (struct sockaddr *)&cliaddr, &cliaddr_len);
  if (conn_fd < 0) {
    ERROR_MSG("accept client link error! [%d][%s]", errno, strerror(errno));
    return -1;
  }

  //创建conn连接信
  ST_SockConnHandle *client_ptr=alloc_conn_info();//申请一个socket连接句柄
  if(NULL==client_ptr)
  {
    ERROR_MSG("连接已经达到上限,无法接受新连接!");
    close(conn_fd);
    return -2;
  }
  memset(client_ptr, 0, sizeof(client_ptr));
  client_ptr->fd=conn_fd;

  ST_SocketConnInfo *conn_info_ptr=&client_ptr->conn_info; //conn_ptr=alloc_conn_info();
  if(NULL==conn_info_ptr)//无可用连接
  {
     //此时连接尚未增加到监听,直接关闭即可;
     close(conn_fd);//注意:需要关闭连接
     ERROR_MSG("do_accept_client failed(not found unuse client)[%s:%d]",
                inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
     return -2;
  }

  conn_info_ptr->sci_id=get_conn_id(conn_fd);//暂时使用文件描述符做连接id吧;
  conn_info_ptr->sci_conn_type=SCI_CONN_TYPE_TCP;
  conn_info_ptr->sci_recv_last_net_serial_no=0;//最近接收序号
  conn_info_ptr->sci_snd_last_net_serial_no=0;//最近的发送序号
  conn_info_ptr->sci_conn_timeout=0;//超时间设置为0,即没有超时时间
  
  conn_info_ptr->sci_sock_fd = conn_fd;//文件描述符
  conn_info_ptr->sci_cs_flag=SCI_CS_FLAG_SERVER;//作为服务器
  
  //获取socket本地ip和port、远程ip和port...
  struct sockaddr_in addr;
  socklen_t addr_len=sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  //获取对方ip和端口
  if((rc=getpeername(conn_fd, (struct sockaddr *)&addr, &addr_len))<0)
  {
     ERROR_MSG("do_accept_client: getpeername() failed, rc=[%d], errno=[%d], strerr=[%s]", rc, errno, strerror(errno));
     close(conn_fd);//注意:需要关闭连接
     conn_fd=0;
     free_conn_info(client_ptr);
     client_ptr=NULL;
     return -3;
  }
  inet_ntop(addr.sin_family, &addr.sin_addr, (char*)conn_info_ptr->sci_remote_ip, sizeof(conn_info_ptr->sci_remote_ip));//对方ip
  conn_info_ptr->sci_remote_port = ntohs(addr.sin_port);
  
  addr_len=sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  if((rc=getsockname(conn_fd, (struct sockaddr *)&addr, &addr_len))<0)//获取本地端口和ip
  {
     ERROR_MSG("do_accept_client: getsockname() failed, rc=[%d], errno=[%d], strerr=[%s]", rc, errno, strerror(errno));
     close(conn_fd);//注意:需要关闭连接
     conn_fd=0;
     free_conn_info(client_ptr);
     client_ptr=NULL;
     return -4;
  }
  inet_ntop(addr.sin_family, &addr.sin_addr, (char *)conn_info_ptr->sci_local_ip, sizeof(conn_info_ptr->sci_local_ip));//本地ip
  conn_info_ptr->sci_local_port = ntohs(addr.sin_port);//本地端口

  //初始化时间...
  os_get_timeval(&conn_info_ptr->sci_start_time);//设置连接建立时间
  conn_info_ptr->sci_last_recv_time = conn_info_ptr->sci_start_time;//上次收到协议包时间
  conn_info_ptr->sci_last_snd_time = conn_info_ptr->sci_start_time;//上次发送协议包时间

  //设置统计信息...
  ST_SocketConnStatics *conn_static_ptr=&conn_info_ptr->sci_conn_statics;//获取统计信息指针
  conn_static_ptr->scs_recv_netpack_count=conn_static_ptr->scs_snd_netpack_count=0;//协议包接收计数与协议包发送计数;
  conn_static_ptr->scs_avg_load=0;//队列平均负载;
  conn_static_ptr->scs_max_load=0;//队列最大负载;
  conn_static_ptr->scs_cur_load=0;//队列当前负载;
  conn_static_ptr->scs_cur_load=0;//队列当前负载;
  conn_static_ptr->scs_support_max_load=0;//支持的最大负载

  //协议包处理时间,统计协议包request/response的耗时
  conn_static_ptr->scs_max_cost_time=0;//最大耗时时间,单位:毫秒
  conn_static_ptr->scs_avg_cost_time=0;//平均耗时时间,单位:毫秒
  conn_static_ptr->scs_cur_cost_time=0;//最近若干协议包处理耗时时间,单位:毫秒

  //添加到连接池
  //CSocketConnPool *conn_pool_ptr=mxx_get_socket_conn_pool(ConnPool_RCV);
  //conn_pool_ptr->set_socketConnInfo(conn_ptr->sci_id, conn_ptr);
  
  return conn_info_ptr->sci_sock_fd;
FAILED:
  if(NULL!=client_ptr)
  {
    free_conn_info(client_ptr);
    client_ptr=NULL;
  }
  if(conn_fd>0)
  {
    close(conn_fd);
    conn_fd=0;
  }

  return ret_code;

}

//功能:连接被关闭处理事件,但此处不执行关闭操作,调用该函数前一定注意关闭socket fd
int do_disconn(int so, bool *close_flag)
{
   if(NULL!=close_flag) *close_flag=false;

   ST_SockConnHandle *ptr=get_conn_info(so);
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
  //unsigned short m_lstn_port = 12345;
  //unsigned int so_concurrency=100;//连接并发数
  
  //epoll配置信息
  int epoll_max_size=MAX_EPOLL_SIZE; //epoll句柄监听最大文件数
  int epoll_wait_timeout=5000;//epoll_wait超时时间,单位毫秒; 0-立即返回; -1不确定;

  
  
  //启动监听端口
  if((fd_listen=socket(AF_INET, SOCK_STREAM, 0))<0)
  {
     FATAL_MSG("thread_recv: create socket  for server port error!");
     return -4;
  }

  INFO_MSG("thread_recv: success to create socket server!\n");

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

  INFO_MSG("thread_recv: success to bind server port[%d]!\n", m_lstn_port);

  if(listen(fd_listen, m_max_listen)<0)
  {
     close(fd_listen);
     fd_listen=0;
     FATAL_MSG("thrread_recv: listen(%d) failed!", m_lstn_port);
     return -6;
  }

  INFO_MSG("thread_recv: sucess to listen server port!\n");

  /*NOTE_WARN_0000: 连接对方崩溃,则epoll没法检测出来,需要在业务层想法使用心跳*/
  //创建epoll句柄
  fd_epoll = epoll_create(epoll_max_size);
  if (fd_epoll < 0) {
    close(fd_listen);
    fd_listen=0;
    FATAL_MSG("thread_recv: create epoll failed.%d\n", fd_epoll);
    return -1;
  }

  INFO_MSG("thread_recv: success to create epoll handle!\n");

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

  INFO_MSG("thread_recv: success to add server socket to epoll monitor!\n");


  g_client_ptr = new ST_SockConnHandle[m_max_listen+1];//客户端连接
  if(NULL==g_client_ptr)
  {
      close(fd_epoll);
      close(fd_listen);
      fd_epoll = -1;
      fd_listen = -1;
      FATAL_MSG("创建连接池失败!");
      return -1005;
  }

  struct epoll_event events[MAX_EPOLL_SIZE];//监听最大事件数量
  int epoll_sig_interrpt_count=0;//epoll被signal中断次数
  bool conn_close_flag=false;//连接断开标记
  while(!m_stop_flag) {
   
    TEST_MSG("start to epoll_wait...\n");
    int nfds = epoll_wait(fd_epoll, events, MAX_EPOLL_SIZE, epoll_wait_timeout);
    TEST_MSG("epoll_wait return fd count[%d]... \n\n", nfds);

    if (nfds < 0) 
    {
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
  if(NULL!=g_client_ptr)
  { 
     delete [] g_client_ptr;
     g_client_ptr=NULL;
  }

  //关闭各个线程

  //关闭日志
  return 0;
}
