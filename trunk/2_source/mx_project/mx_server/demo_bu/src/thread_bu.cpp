
#include <string.h>
#include <errno.h>

#include "logfile.h"
#include "ConfigFile.h"

#include "thread_bu.h"
#include "servercontext.h"

//构造函数
CBuThread::CBuThread(const char *thread_name/*="recv_thread"*/)
    :Thread_Base(thread_name),m_b_running(false),m_stop_flag(false)
{
   memset(m_bcc_ip, 0, sizeof(m_bcc_ip);
   m_bcc_port = -1;
   m_recv_timeout = m_send_timeout = 0; 
   m_bccconn_so = -1;   //!< socket
   
   m_b_running=false;
   m_stop_flag=false;
}
//析构函数
CBuThread::~CBuThread()
{
}

int CBuThread::init()
{
   return 0;
}

int CBuThread::terminate_service()
{
    m_stop_flag=true;
    return 0;
}

void CBuThread::run()
{
    m_b_running=true;
	this->listen_routine();
	m_b_running=false;
}
int CBuThread::loadini(const char *cfgfile)
{
    ConfigFile cfg;
    int rc=cfg.load_cfg_file(cfgfile);
    if(rc < 0)
    {
        ERROR_MSG("打开配置文件失败, rc=[%d]", rc);
        return -1;
    }
    
    char serve_section[]="sourth_server";
    m_lstn_port   =cfg.read_int(serve_section,  "backend_listen_port", 9999);//!< 服务监听端口
    m_max_listen  =cfg.read_int(serve_section,  "backend_max_conn",    1024); //!< 最大连接数
    m_recv_timeout=cfg.read_int(serve_section,  "backend_timeout",     1000); //!< 接收超时,单位毫秒
    m_send_timeout = m_recv_timeout; //!< 发送超时时间,单位毫秒
    
    if(m_lstn_port<=0)
    {
        FATAL_MSG("[%s][%s]服务端口配置不合法!", serve_section, "backend_listen_port");
        return -1;
    }
    if(m_max_listen<1)
    {
        FATAL_MSG("[%s][%s]最大连接数配置不合法!", serve_section, "backend_max_conn");
        return -1;
    }
    
    if( ((m_recv_timeout<0)||(m_recv_timeout>3000))
        ||((m_send_timeout<0)||(m_send_timeout>3000))
      )
    {
        FATAL_MSG("[%s][%s]超时时间配置不合法!范围[0,3000]", serve_section, "backend_timeout");
        return -1;
    }
    
    return 0;   
}

int CBuThread::listen_routine() 
{
  int rc;  
  
  //启动监听端口
  if((m_listen_so=socket(AF_INET, SOCK_STREAM, 0))<0)
  {
     FATAL_MSG("listen_thread: create socket  for server port error!");
     return -4;
  }

  INFO_MSG("listen_thread: success to create socket server!\n");

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family=AF_INET;
  server_addr.sin_port=htons(m_lstn_port);
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(m_listen_so, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
  {
     close(m_listen_so);
     m_listen_so=-1;
     FATAL_MSG("listen_thread: bind port[%d] failed!", m_lstn_port);
     return -5;
  }

  INFO_MSG("listen_thread: success to bind server port[%d]!\n", m_lstn_port);

  if(listen(m_listen_so, m_max_listen)<0)
  {
     close(m_listen_so);
     m_listen_so=-1;
     FATAL_MSG("listen_thread: listen(%d) failed!", m_lstn_port);
     return -6;
  }

  INFO_MSG("listen_thread: sucess to listen server port\n");

  //业务比较少,使用select即可
  //设置socket读超时时间,感觉有点不靠谱
  fd_set fdset;//
  struct timeval tv;//!< 超时时间,如果accept不设置超时时间, 则一直阻塞,会收不到进程停止信号

  INFO_MSG("listen_thread: success to add server socket to epoll monitor!\n");


  //g_client_ptr = new ST_SockConnHandle[m_max_listen+1];//客户端连接
  //if(NULL==g_client_ptr)
  //{
  //    close(fd_epoll);
  //    close(fd_listen);
  //    fd_epoll = -1;
  //    fd_listen = -1;
  //    FATAL_MSG("创建连接池失败!");
  //    return -1005;
  //}

  //struct epoll_event events[MAX_EPOLL_SIZE];//监听最大事件数量
  //int epoll_sig_interrpt_count=0;//epoll被signal中断次数
  //bool conn_close_flag=false;//连接断开标记

  int errcode;
  char szMsg[256]={0};
  while(!m_stop_flag) 
  {
    FD_ZERO(&fdset);
	tv.tv_sec = 5;//!< 超时时间5秒; 懒得读取配置文件
	tv.tv_usec=0;
   
    //nfds=最大文件描述符+1
	//accept需要读操作触发
    rc=select(m_listen_so+1, &fdset, NULL, NULL, &tv);
	if(rc<0)
	{
	   errcode=errno;
	   if(EINTR==errcode)
		   continue;
	   else
	   {
		   ERROR_MSG("CBuThread::listen_routine select errro! [%d][%s]", errcode, strerror(errcode));
	   }
	   continue;
	}
	else if(0==rc)
	{
		DEBUG_MSG("no client accept");
		continue;
	}
	//select 返回的是触发描述符个数,不过此处只有一个,也就不用检查了
	
	struct sockaddr_in cliaddr;//客户端地址;
	socklen_t cliaddr_len = sizeof(cliaddr);
	memset(&cliaddr, 0, sizeof(cliaddr));
	int conn_fd = accept(m_listen_so, (struct sockaddr *)&cliaddr, &cliaddr_len);
	if (conn_fd < 0) 
	{
	   errcode=errno;
      ERROR_MSG("listen_thread: accept client link error! [%d][%s]", errcode, strerror(errcode));
      continue;//return -1;//也不清楚该怎么处理了???
    }
	
	//todo 查找可用连接吧!
	
	//todo 启动服务...
	CServerContext *ctx_instance=CServerContext::get_instance();
	if(NULL==ctx_instance)
	{
		ERROR_MSG("no server context,can not bing socket to bulinkthread");
		close(conn_fd);
		continue;
	}

	//将socket绑定到一个业务线程
	rc=ctx_instance->bind_socket_to_bulinkthread(conn_fd, szMsg);
	if(rc<0)
	{
		ERROR_MSG("bind socket to bulinkthread error! [%d] [%s]", rc, szMsg);
		close(conn_fd);
		continue;
	}

  }//继续等待

  //清除连接
  close(m_listen_so);
  m_listen_so=-1;

  return 0;
}
