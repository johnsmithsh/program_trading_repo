

#include "thread_bulink.h"
#include "logfile.h"

//构造函数
CBuLinkThread::CBuLinkThread(const char *thread_name/*="recv_thread"*/)
    :Thread_Base(thread_name), m_sock_fd(MXX_SOCKET_INVALID_FD),m_max_listen(0),m_recv_timeout(0),m_send_timeout(0),m_b_running(false),m_stop_flag(false)
{
   m_max_listen=0;
   m_recv_timeout=m_send_timeout=0;
   m_b_running=false;
   m_stop_flag=false;
}
//析构函数
CBuLinkThread::~CBuLinkThread()
{
    if(m_sock_fd>0)
	{
	    close(m_sock_fd);
		m_sock_fd=MXX_SOCKET_INVALID_FD;
	}
}

int CBuLinkThread::init()
{
   return 0;
}

int CBuLinkThread::terminate_service()
{
    m_stop_flag=true;
	m_link_stat = LNK_STAT_TERMINATE;//!< 设置为结束状态
	if(m_sock_fd>0)
	{
	    close(m_sock_fd);//会导致socket函数退出;
		m_sock_fd=MXX_SOCKET_INVALID_FD;
	}
    return 0;
}

void CBuLinkThread::run()
{
    m_b_running=true;
	this->service_routine();
	m_b_running=false;
}

int CBuLinkThread::loadini(char *cfgfile)
{
    ConfigFile cfg;
    int rc=cfg.load_cfg_file(cfgfile);
    if(rc < 0)
    {
        ERROR_MSG("打开配置文件失败, rc=[%d]", rc);
        return -1;
    }
    
    char serve_section[]="sourth_server";
    //m_lstn_port   =cfg.read_int(serve_section,  "serverport", 9999);//!< 服务监听端口
    m_max_listen  =cfg.read_int(serve_section,  "max_conn",   1024); //!< 最大连接数
    m_recv_timeout=cfg.read_int(serve_section,  "timeout",    1000); //!< 接收超时,单位毫秒
    m_send_timeout = m_recv_timeout; //!< 发送超时时间,单位毫秒
    
    //if(m_lstn_port<=0)
    //{
    //    FATAL_MSG("[%s][%s]服务端口配置不合法!", serve_section, "serverport");
    //    return -1;
    //}
    //if(m_max_listen<1)
    //{
    //    FATAL_MSG("[%s][%s]最大连接数配置不合法!", serve_section, "max_conn");
    //    return -1;
    //}
    
    if( ((m_recv_timeout<0)||(m_recv_timeout>3000))
        ||((m_send_timeout<0)||(m_send_timeout>3000))
      )
    {
        FATAL_MSG("[%s][%s]超时时间配置不合法!范围[0,3000]", serve_section, "timeout");
        return -1;
    }
    
    return 0;   
}

int CBuLinkThread::bind_to_socket(int so)
{
    //其实只有listen_thread调用该函数,不会出现竞争,就不用线程锁了
    if(so<=0)//socket无效
	    return -1;
		
	//判断服务状态...
	if(LNK_STAT_INIT==m_link_stat)//服务没有运行,先加载配置文件
	{
	    rc=this->loadini();
		if(rc<0)
		{
		    ERROR_MSG("link_thread: 加载配置文件失败! rc=[%d]", rc);
			return -4;
		}
	}
	else if(LNK_STAT_TERMINATE==m_link_stat)//服务停止指令,不能再绑定新
	{
	    return -2;
	}
	else if(m_link_stat>=LNK_STAT_LINKING)//已经有连接了,禁止绑定新业务
	{
	    return -3;
	}
	
	
	//先把状态设置了,防止被强占; 其实只有listen_thread调用该函数,不会出现竞争,就不用线程锁了
	m_link_stat=LNK_STAT_LINKING;
	m_sock_fd = so;
	
	this->start_thread();//!< 启动线程
	return 0;
}

int CBuLinkThread::service_routine() 
{
  int rc;  
  
  INFO_MSG("listen_thread: sucess to listen server port\n");

  //业务比较少,使用select即可
  //设置socket读超时时间,感觉有点不靠谱
  fd_set fdset;//
  struct timeval tv;//!< 超时时间,如果accept不设置超时时间, 则一直阻塞,会收不到进程停止信号

  INFO_MSG("listen_thread: success to add server socket to epoll monitor!\n");

  int errcode;
  while((!m_stop_flag) //线程停止指令
         ||(LNK_STAT_TERMINATE==m_sock_fd) //服务停止指令
		 ||(LNK_STAT_INIT==m_sock_fd) //线程尚未运行
	   )
  {
    if(LNK_STAT_NONE_SERVICE==m_link_stat)//没有服务接入
	{
	    //if(m_sock_fd>0) //没有服务接入, socket运行,是个不正常状态
		//{
		//    close(m_sock_fd);
		//	m_sock_fd=MXX_SOCKET_INVALID_FD;
		//}
	    os_thread_msleep(1000);//
	    continue;
	}
	else if(LNK_STAT_LINKING==m_link_stat)//!< 正在连接...
	{
	    //todo 接收注册信息...
		continue;
	}
	else if( (LNK_STAT_IDLE==m_link_stat)//!< 空闲
	         ||(LNK_STAT_HIGH_LOAD==m_link_stat) //!< 高负载
		   )
	{
	    // todo 接收数据 只有一个连接采用select即可
	}
	else
	{
	}
	
    FD_ZERO(&fdset);
	tv.tv_sec = 5;//!< 超时时间5秒; 懒得读取配置文件
	tv.tv_usec=0;
   
    //nfds=最大文件描述符+1
	//accept需要读操作触发
    rc=select(m_sock_fd+1, &fdset, NULL, NULL, &tv);
	if(rc<0)
	{
	   errcode=errno;
	}
	else if(0==rc)
	{
	}
	//select 返回的是触发描述符个数,不过此处只有一个,也就不用检查了
	
	struct sockaddr_in cliaddr;//客户端地址;
	socklen_t cliaddr_len = sizeof(cliaddr);
	memset(cliaddr, 0, sizeof(cliaddr));
	int conn_fd = accept(fd_listen, (struct sockaddr *)&cliaddr, &cliaddr_len);
	if (conn_fd < 0) 
	{
	  errcode=errno;
      ERROR_MSG("listen_thread: accept client link error! [%d][%s]", errcode, strerror(errcode));
      continue;//return -1;//也不清楚该怎么处理了???
    }
	
	//todo 查找可用连接吧!
	
	//todo 启动服务...

  }//继续等待

  //清除连接
  close(fd_listen);
  fd_listen=0;

  return 0;
}

int CBuLinkThread::register_link_info(char *buff, char *buffsize, char *data_len)
{
    
}

int CBuLinkThread::recv_link_info(char *buff, char *buffsize, char *data_len)
{
    
}