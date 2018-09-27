
#include <errno.h>
#include <stdio.h>

#include "mxx_net_socket.h"
#include "thread_bulink.h"
#include "logfile.h"
#include "ConfigFile.h"
#include "msg_link_function.h"
#include "bufuncmanage.h"
#include "servercontext.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
extern int msglink_send(int so, const unsigned char *buff, size_t data_len, char *errmsg);
extern int msglink_recv(int so, unsigned char *buff, size_t buffsize, int *data_len, char *err_msg);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        ERROR_MSG("打开配置文件[%s]失败, rc=[%d]", cfgfile, rc);
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

int CBuLinkThread::clear_buinfo()
{
    m_group_id=MXX_BUGROUPID_INVALID;
	memset(m_group_no, 0, sizeof(m_group_no));
	memset(m_buversion, 0, sizeof(m_buversion));
	memset(m_buprogname, 0, sizeof(m_buprogname));
	memset(m_bu_pid, 0, sizeof(m_bu_pid));
	m_bu_no = MSG_BUNO_INVALID;
	return 0;
}

void CBuLinkThread::get_bulinkinfo(ST_SVR_LINK_HANDLE &bulinkinfo)
{
	memset(&bulinkinfo, 0, sizeof(ST_SVR_LINK_HANDLE));

	CServerContext *context_ptr=CServerContext::get_instance();//一般不会返回NULL
	bulinkinfo.link_info.bcc_id =context_ptr->get_bcc_id();
	bulinkinfo.link_info.bu_no  = m_bu_no;
	strncpy(bulinkinfo.link_info.group_no, m_group_no, sizeof(bulinkinfo.link_info.group_no)-1);
	//bulinkinfo.link_info.group_desc;
	//bulinkinfo.link_info.link_mode;
	//bulinkinfo.link_info.link_role;

	bulinkinfo.so = m_sock_fd;
	//bulinkinfo.recv_serial;
	//bulinkinfo.send_serial;
	//bulinkinfo.start_timestamp;
	//bulinkinfo.version;
}

int CBuLinkThread::clear_sockinfo()
{
    mxx_socket_delete(m_sock_fd);
	m_sock_fd=-1;
	m_link_stat=LNK_STAT_NONE_SERVICE;
}

int CBuLinkThread::bind_to_socket(int so)
{
    //其实只有listen_thread调用该函数,不会出现竞争,就不用线程锁了
    if(so<=0)//socket无效
	    return -1;
	int rc=0;	
	//判断服务状态...
	if(LNK_STAT_INIT==m_link_stat)//服务没有运行,先加载配置文件
	{
	    rc=this->loadini(NULL);
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
         //||(LNK_STAT_TERMINATE==m_link_stat) //服务停止指令
		 ||(LNK_STAT_INIT==m_link_stat) //线程尚未运行
	   )
  {
    if(LNK_STAT_NONE_SERVICE==m_link_stat)//没有服务接入,线程处于空转状态
	{
	    //if(m_sock_fd>0) //没有服务接入, socket运行,是个不正常状态
		//{
		//    close(m_sock_fd);
		//	m_sock_fd=MXX_SOCKET_INVALID_FD;
		//}
	    os_thread_msleep(1000);//
	    continue;
	}
	
    //等待业务进程数据...
	FD_ZERO(&fdset);
	tv.tv_sec = 5;//!< 超时时间5秒; 懒得读取配置文件
	tv.tv_usec=0;
   
    //nfds=最大文件描述符+1
	//accept需要读操作触发
    rc=select(m_sock_fd+1, &fdset, NULL, NULL, &tv);
	if(rc<0)
	{
	   errcode=errno;
	   continue;
	}
	else if(0==rc)
	{
	}
	
	//select 返回的是触发描述符个数,不过此处只有一个,也就不用检查了
	if(!FD_ISSET(m_sock_fd, &fdset))//不是当前socket触发的,继续等待
	    continue;
	
	
	switch(m_link_stat)
	{
	  case LNK_STAT_LINKING:      //!< 正在连接...
	  {
	      rc=wait_buconn();
		  break;
	  }
	  case LNK_STAT_REGISTERING: //!< 正在注册业务
	  {
	      rc=wait_buregister();
		  break;
	  }
	  case LNK_STAT_IDLE:      //!< 空闲
	  case LNK_STAT_HIGH_LOAD: //!< 高负载
	  {
	     //处理业务消息
         rc=process_msg();
		 break;
	  }
	  default:
	  {
	      //todo 不支持
		  break;
	  }
	}
	
	if(rc<0)//处理错误,线程准备退出
	{
	    break;
	}
	//后续处理
    
	//if(LNK_STAT_LINKING==m_link_stat)//!< 正在连接...
	//{
	//    //todo 接收注册信息...
	//	rc=wait_bulink();
	//	continue;
	//}
	//else if(LNK_STAT_REGISTERING==m_link_stat)//!< 正在注册业务
	//{
	//    wait_buregister();
	//    continue;
	//}
	//else if( (LNK_STAT_IDLE==m_link_stat)//!< 空闲
	//         ||(LNK_STAT_HIGH_LOAD==m_link_stat) //!< 高负载
	//	   )
	//{
	//    // todo 接收数据 只有一个连接采用select即可
	//}
	//else
	//{
	//    break;//退出服务
	//}
	
    

  }//继续等待

  //清除连接
  close(m_sock_fd);
  m_sock_fd=-1;

  return 0;
}

//等待业务进程连接
int CBuLinkThread::wait_buconn()
{
    char szmsg[255]={0};
    //接收消息,确认已经有消息过来了
    ST_MSGLINK_BUFF msg_buff;
	int data_len=0;
    int rc=msglink_recv(m_sock_fd, (unsigned char *)&msg_buff, sizeof(msg_buff), &data_len, szmsg);
	if(rc<0)//接收错误
	{
	    return -1;
	}
	
	if(msglink_check_ccflag((unsigned char *)&msg_buff, CC_ACK_FLAG))//ACK标记,不用做后续处理
	{
	    return 0;
	}
	
	//根据不同的消息类型做不同的处理
	ST_MSG_HEAD   *head_ptr=&msg_buff.head;//报文头
	ST_MSG_COMMON *info_ptr=&msg_buff.commoninfo;//msg消息头
	if(MSGTYPE_CONN!=head_ptr->msgid)//必须连接请求报文
	{
	    return -1;
	}
	rc=process_bulink(&msg_buff);
	return rc;
}

//等待业务进程注册业务功能
int CBuLinkThread::wait_buregister()
{
    char szmsg[255]={0};
    //接收消息,确认已经有消息过来了
    ST_MSGLINK_BUFF msg_buff;
	int data_len=0;
    int rc=msglink_recv(m_sock_fd, (unsigned char *)&msg_buff, sizeof(msg_buff), &data_len, szmsg);
	if(rc<0)//接收错误
	{
	    return -1;
	}
	
	if(msglink_check_ccflag((unsigned char *)&msg_buff, CC_ACK_FLAG))//ACK标记,不用做后续处理
	{
	    return 0;
	}
	
	//根据不同的消息类型做不同的处理
	ST_MSG_HEAD   *head_ptr=&msg_buff.head;//报文头
	ST_MSG_COMMON *info_ptr=&msg_buff.commoninfo;//msg消息头
	if(MSGTYPE_REG_FUNC!=head_ptr->msgid)//必须连接请求报文
	{
	    return -1;
	}
	rc=process_buregister(&msg_buff);
	return rc;
}

//处理业务消息
int CBuLinkThread::process_msg()
{
    char szmsg[255]={0};
    //接收消息,确认已经有消息过来了
    ST_MSGLINK_BUFF msg_buff;
	int data_len=0;
    int rc=msglink_recv(m_sock_fd, (unsigned char *)&msg_buff, sizeof(msg_buff), &data_len, szmsg);
	if(rc<0)//接收错误
	{
	}
	
	if(msglink_check_ccflag((unsigned char *)&msg_buff, CC_ACK_FLAG))//ACK标记,不用做后续处理
	{
	    return 0;
	}
	
	//根据不同的消息类型做不同的处理
	ST_MSG_HEAD   *head_ptr=&msg_buff.head;//报文头
	ST_MSG_COMMON *info_ptr=&msg_buff.commoninfo;//msg消息头
    switch(head_ptr->msgid)//根据不同消息id做不同处理
	{
	    case MSGTYPE_CONN: //建立连接; 业务进程=>控制中心
		{
			rc=process_bulink(&msg_buff);
			if(rc<0)
			{
			}
			
			break;
		}
		case MSGTYPE_DISCONN://断开连接 业务进程<=>控制中心
		{
		    break;
		}
		case MSGTYPE_REG_FUNC://注册业务 业务进程=>控制中心, 业务进程向控制中心注册支持的业务
		{
		    rc=process_buregister(&msg_buff);
			if(rc<0)
			{
			}
		    break;
		}
		case MSGTYPE_HEARTBEAT://心跳
		{
		    break;
		}
		case MSGTYPE_DATA://数据传输
		{
		    break;
		}
		case MSGTYPE_REQ_REQUEST://业务请求 业务进程<=控制中心; 控制中心向业务进程分配任务
		{
		    break;
		}
		case MSGTYPE_ACK_REQUEST://请求确认包 控制中心<=业务进程: 业务进程向控制中心表明已收到业务数据;
		{
		    break;
		}
		case MSGTYPE_REQ_RESPONSE://请求应答包
		{
		    break;
		}
		case MSGTYPE_REQ_TRANSFER://数据传送 控制中心<=业务进程: 业务进程向控制进程发送业务数据
		{
		    rc=process_butransfer(&msg_buff);
			if(rc<0)
			{
			}
		    break;
		}
		default:
		{
		    break;
		}
	}// end of switch
	
	
    return -1;
}

/**@brief 校验消息头
 * @param
 *  
 **/
int CBuLinkThread::check_msg_info(ST_MSGLINK_BUFF *msg_buff_ptr, unsigned short msgid)
{
    if(NULL==msg_buff_ptr)
	    return -1;
	
    //msg公共消息是否存在
	if(msg_buff_ptr->head.data_len<sizeof(ST_MSG_COMMON)) //!< 缺少信息头,不用继续处理了
	{
	    ERROR_MSG("msgid=[0x%02x], lack of msg common info", msg_buff_ptr->head.msgid);
		return -1;
	}
	
	ST_MSG_HEAD   *head_ptr=&msg_buff_ptr->head;//!< 报文头
	ST_MSG_COMMON *info_ptr=&msg_buff_ptr->commoninfo;//!< msg公共消息
    //msgid与预期不一致
	if(msgid != msg_buff_ptr->head.msgid)
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], lack of msg connect package[0x%02x]", head_ptr->msgid, info_ptr->group_no, MSGTYPE_CONN);
		return -1;
	}
	
	return 0;
}

//@brief 连接请求: 控制中心处理业务进程发起连接请求;
int CBuLinkThread::process_bulink(ST_MSGLINK_BUFF *msg_buff_ptr)
{	
	int err_code=0;
	char szmsg[255]={0};
	int rc=0;
	ST_MSG_HEAD   *head_ptr=&msg_buff_ptr->head;//!< 报文头
	ST_MSG_COMMON *info_ptr=&msg_buff_ptr->commoninfo;//!< msg公共消息
	MSG_REQ_CONN *conn_ptr=(MSG_REQ_CONN *)msg_buff_ptr->data_buff;//!< msg数据指针
	if(head_ptr->data_len<sizeof(ST_MSG_COMMON)) //!< 缺少信息头,不用继续处理了
	{
	    ERROR_MSG("msgid=[0x%02x], lack of msg common info", head_ptr->msgid);
		return -1;
	}
	if(MSGTYPE_CONN != head_ptr->msgid)//!< 不是msg连接请求,返回错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], group_desc=[%s], lack of msg connect package[0x%02x]", head_ptr->msgid, info_ptr->group_no, conn_ptr->group_desc, MSGTYPE_CONN);
		return -1;
	}
	else if(head_ptr->data_len<sizeof(ST_MSG_COMMON)+sizeof(MSG_REQ_CONN))//!< 缺失连接请求数据,返回错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], group_desc=[%s], lack of bu connect information", head_ptr->msgid, info_ptr->group_no, conn_ptr->group_desc);
		return -1;
	}
	
	//连接时没有上传业务组号,则返回错误
	if('\0'==conn_ptr->group_no[0])
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], group_desc=[%s], lack of group no", head_ptr->msgid, info_ptr->group_no, conn_ptr->group_desc);
		return -1;
	}
	
	
	CServerContext *context_ptr=CServerContext::get_instance();//一般不会返回NULL
	
	CBuGroupInfo *group_info_ptr=CFuncRegister::find_groupinfo((char *)conn_ptr->group_no);//context_ptr->find_groupinfo(conn_ptr->group_no);
	if(NULL==group_info_ptr)//业务组不存在,则增加业务组信息
	{
		//没有业务组,则增加新的业务组
		ST_BUGROUP_ITEM group_item;
		memset(&group_item, 0, sizeof(group_item));
		strncpy((char*)group_item.group_no,    conn_ptr->group_no, sizeof(group_item.group_no)-1);  //!< 业务组号, 业务进行分类,由业务进程连接时上传
      //unsigned char group_name[32];//!< 组名, 由业务进程连接时上传
      strncpy(group_item.prog_name, conn_ptr->prog_name, sizeof(group_item.prog_name)-1); //!< 业务程序名,由业务进程连接时上传
		rc=CFuncRegister::reginfo_add_group_info(&group_item);//!< 增加新的业务组
		if(rc<0)
		{
		    ERROR_MSG("msgid=[0x%02x], group_no=[%s], group_desc=[%s], failed to add new group", head_ptr->msgid, info_ptr->group_no, conn_ptr->group_desc);
			return -1;
		}
		
		group_info_ptr=CFuncRegister::find_groupinfo((char *)conn_ptr->group_no);
		if(rc<0)//!< 不应该走到此处
		{
		    ERROR_MSG("msgid=[0x%02x], group_no=[%s], group_desc=[%s], add group info succ,but can not find group info", head_ptr->msgid, info_ptr->group_no, conn_ptr->group_desc);
			return -1;
		}
	}
	
	
	//更新bu业务信息...
	m_group_id=group_info_ptr->get_groupid();//!< (控制中心分配)业务组id
	
	memset(m_group_no, 0, sizeof(m_group_no));
	strncpy(m_group_no,conn_ptr->group_no,sizeof(m_group_no)-1);//!< 业务组号
	
	memset(m_buversion, 0, sizeof(m_buversion));
	memcpy(m_buversion, conn_ptr->version, sizeof(conn_ptr->version));//!< 版本信息
	
	snprintf(m_bu_pid, sizeof(m_bu_pid)-1, "%ld", conn_ptr->proc_id);//!< 业务进程id
	
	int bu_no=context_ptr->get_next_buno();//生成新bu_no
	m_bu_no=bu_no;
	
	//应答...
	ST_SVR_LINK_HANDLE bulinkinfo;
	get_bulinkinfo(bulinkinfo);

	ST_MSGLINK_BUFF msg_rsp_buff;
	int bcc_id=context_ptr->get_bcc_id();//获取当前控制中心id
	lmasm_ans_connect(&msg_rsp_buff, &bulinkinfo, bcc_id, m_bu_no, C_YES, "", NULL); //!<构建应答报文

	////设置msg id
	//msglink_pkg_head_init((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), MSGTYPE_CONN, szmsg);
	////设置连接信息
	//msglink_pkg_conninfo((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), bcc_id, bu_no,conn_ptr->group_no, szmsg);
	////设置控制信息
	//bool first_flag=true,next_flag=false, ack_flag=true,push_flag=false;
	//msglink_pkg_ctrlinfo((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), first_flag, next_flag, ack_flag, push_flag, szmsg);
	//
	//
	////增加业务数据
	//MSG_ANS_CONN rsp_data;
	//memset(&rsp_data, 0, sizeof(rsp_data));
	//rsp_data.bu_no=bu_no;
	//rsp_data.bcc_id=bcc_id;
	//rsp_data.if_succ=C_YES;//成功标记
	//msglink_pkg_data_append((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), (unsigned char *)&rsp_data, sizeof(rsp_data), szmsg);
	

	//发送数据
	rc=msglink_send(m_sock_fd, (unsigned char *)&msg_rsp_buff, msg_rsp_buff.head.data_len+sizeof(msg_rsp_buff.head), szmsg);
	if(rc<0)//接收错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], send response msg for connection failed!,rc=[%d]", head_ptr->msgid, info_ptr->group_no, conn_ptr->group_desc, rc);
		return -1;
	}
	
	m_link_stat=LNK_STAT_REGISTERING;//修改连接状态
	return 0;
}

//@brief 处理(业务进程|控制中心)发起断开连接请求;
int CBuLinkThread::process_disconn(ST_MSGLINK_BUFF *msg_buff_ptr)
{
    int err_code=0;
	char szmsg[255]={0};
	ST_MSG_HEAD   *head_ptr=&msg_buff_ptr->head;//!< 报文头
	ST_MSG_COMMON *info_ptr=&msg_buff_ptr->commoninfo;//!< msg公共消息
	unsigned char *data_ptr=(unsigned char *)msg_buff_ptr->data_buff;//!< msg数据指针
	
	//校验是否是预期消息
	int rc=check_msg_info(msg_buff_ptr, MSGTYPE_DISCONN);
	if(rc<0)
	{
	    return -1;
	}
	m_link_stat=LNK_STAT_DISCONNING;//!< 修改连接状态

	//ACK消息不用处理,直接返回即可
	//if(msglink_check_ccflag((unsigned char *)&msg_buff, CC_ACK_FLAG))
	//{
	//    return 0;
	//}
	
	CServerContext *context_ptr=CServerContext::get_instance();//一般不会返回NULL
	CBuGroupInfo *group_info_ptr=CFuncRegister::find_groupinfo((char *)info_ptr->group_no);
	
	//应答... 先构建应答数据
	ST_MSGLINK_BUFF msg_rsp_buff;
	
	int bcc_id=context_ptr->get_bcc_id();//获取当前控制中心id
	ST_SVR_LINK_HANDLE bulinkinfo;
	get_bulinkinfo(bulinkinfo);
	lmasm_ans_disconnection(&msg_rsp_buff, &bulinkinfo, C_YES, "", NULL); //!<构建应答报文
	
	////设置msg id
	//msglink_pkg_head_init((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), MSGTYPE_DISCONN, szmsg);
	////设置连接信息
	//msglink_pkg_conninfo((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), bcc_id, m_bu_no, info_ptr->group_no, szmsg);
	////设置控制信息
	//bool first_flag=true,next_flag=false, ack_flag=true,push_flag=false;
	//msglink_pkg_ctrlinfo((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), first_flag, next_flag, ack_flag, push_flag, szmsg);
	//
	//MSG_RSP rsp_data;//应答信息
	//rsp_data.if_succ=C_YES;
	//msglink_pkg_data_append((unsigned char *)&msg_rsp_buff, sizeof(msg_rsp_buff), (unsigned char *)&rsp_data, sizeof(rsp_data), szmsg);

	//发送数据
	rc=msglink_send(m_sock_fd, (unsigned char *)&msg_rsp_buff, msg_rsp_buff.head.data_len+sizeof(msg_rsp_buff.head), szmsg);
	if(rc<0)//接收错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], send response msg for connection failed!,rc=[%d]", head_ptr->msgid, info_ptr->group_no, rc);
		//return -1;
	}
	
	clear_buinfo();
	clear_sockinfo();
	return 0;
}

//@brief 控制中心处理业务进程发起注册请求;
int CBuLinkThread::process_buregister(ST_MSGLINK_BUFF * msg_buff_ptr)
{	
	//int err_code=0;
	char szmsg[256]={0};
	ST_MSG_HEAD   *head_ptr=&msg_buff_ptr->head;//!< 报文头
	ST_MSG_COMMON *info_ptr=&msg_buff_ptr->commoninfo;//!< msg公共消息
	unsigned char * data_ptr=(unsigned char *)msg_buff_ptr->data_buff;//!< msg数据指针
	
	//校验是否是预期消息
	int rc=check_msg_info(msg_buff_ptr, MSGTYPE_REG_FUNC);
	if(rc<0)
	{
	    return -1;
	}
	
	if(head_ptr->data_len<sizeof(ST_MSG_COMMON)+sizeof(MSG_REQ_REGFUNC))//!< 缺失msg数据,返回错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], lack of bu connect information", head_ptr->msgid, info_ptr->group_no);
		return -1;
	}
	
	//一般不会返回NULL
	CServerContext *context_ptr=CServerContext::get_instance();
	CBuGroupInfo *group_info_ptr=NULL;//context_ptr->find_groupinfo(conn_ptr->group_no);
	if(NULL!=group_info_ptr)//也可以直接添加新的group no
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], unsupport this group no", head_ptr->msgid, info_ptr->group_no);
		return -1;
	}
	
	//ACK消息不用处理,直接返回即可
	if(msglink_check_ccflag((unsigned char *)msg_buff_ptr, CC_ACK_FLAG))
	{
	    return 0;
	}
	
	//将bu支持的业务注册到本地
	int nsize=(head_ptr->data_len-sizeof(ST_MSG_COMMON))/sizeof(MSG_REQ_REGFUNC);
	unsigned char *ptr=data_ptr;
	for(int i=0; i<nsize; i++)
	{
	    MSG_REQ_REGFUNC* funcinfo_ptr=(MSG_REQ_REGFUNC*)ptr;
		//group_info_ptr->register_func(0, funcinfo_ptr->bu_func_id, funcinfo_ptr->bu_func_desc);//!< 注册业务功能
		
		if('\0'==funcinfo_ptr->bu_func_id[0])
			continue;

   	ST_BUFUNC_DESC func_desc;
   	memset(&func_desc, 0, sizeof(func_desc));
   	strncpy(func_desc.func_id,  funcinfo_ptr->bu_func_id,   sizeof(func_desc.func_id)-1);  //!< 业务功能id,   字符串格式
      strncpy(func_desc.func_name,funcinfo_ptr->bu_name,      sizeof(func_desc.func_name)-1);//!< 功能名
      strncpy(func_desc.func_desc,funcinfo_ptr->bu_func_desc, sizeof(func_desc.func_desc)-1);//!< 业务功能说明
      //char func_prog_name[64];//!< 业务程序名
      func_desc.priority=funcinfo_ptr->priority;
      func_desc.func_comm_type=funcinfo_ptr->bu_func_type;
      rc=CFuncRegister::reginfo_register_func(group_info_ptr->get_groupid(), &func_desc);
		ptr += sizeof(MSG_REQ_REGFUNC);
	}
	
	//应答...
	//构建应答域..
	ST_MSGLINK_BUFF msg_ack_buff;
	int bcc_id =context_ptr->get_bcc_id();//获取当前控制中心id
	ST_SVR_LINK_HANDLE bulinkinfo;
	get_bulinkinfo(bulinkinfo);
	lmasm_ans_register_function(&msg_ack_buff, &bulinkinfo, C_YES, "", NULL); //!<构建应答报文
	
	////设置消息头
	//msglink_pkg_head_init((unsigned char *)&msg_ack_buff, sizeof(msg_ack_buff), MSGTYPE_REG_FUNC, szmsg);//!< 设置msg id
	////连接信息
	//msglink_pkg_conninfo((unsigned char *)&msg_ack_buff, sizeof(msg_ack_buff), bcc_id, m_bu_no,info_ptr->group_no, szmsg);//!< 设置连接信息
	////控制信息
	//bool first_flag=true,next_flag=false, ack_flag=true,push_flag=false;
	//msglink_pkg_ctrlinfo((unsigned char *)&msg_ack_buff, sizeof(msg_ack_buff), first_flag, next_flag, ack_flag, push_flag, szmsg);//!< 设置控制信息
	//
	////增加业务数据
	//MSG_ACK rsp_data;
	//memset(&rsp_data, 0, sizeof(rsp_data));
	//rsp_data.if_succ=C_YES;//成功标记
	//rsp_data.szmsg[0]='\0';
	//msglink_pkg_data_append((unsigned char *)&msg_ack_buff, sizeof(msg_ack_buff), (unsigned char *)&rsp_data, sizeof(rsp_data), szmsg);
	
	//发送数据
	//int data_len=0;
	rc=msglink_send(m_sock_fd, (unsigned char *)&msg_ack_buff, msg_ack_buff.head.data_len+sizeof(msg_ack_buff.head), szmsg);
	if(rc<0)//接收错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], send response msg for connection failed!,rc=[%d]", head_ptr->msgid, info_ptr->group_no, rc);
		return -1;
	}
	
	if(!msglink_check_ccflag((unsigned char *)msg_buff_ptr, CC_NEXT_FLAG))//不存在下一个注册请求
	{
	    m_link_stat=LNK_STAT_IDLE;//服务注册完毕,可以处理业务
	    return 0;
	}
	
	return 0;
}

//@brief 控制中心处理业务进程的应答数据;
// 业务进程处理业务请求后,将处理结果返回;
//
int CBuLinkThread::process_buresponse(ST_MSGLINK_BUFF * msg_buff_ptr)
{
    //int err_code=0;
	ST_MSG_HEAD   *head_ptr=&msg_buff_ptr->head;      //!< 报文头
	ST_MSG_COMMON *info_ptr=&msg_buff_ptr->commoninfo;//!< msg公共消息
	//unsigned char * data_ptr=msg_buff_ptr->data_buff; //!< msg数据指针
	
	//校验是否是预期消息
	int rc=check_msg_info(msg_buff_ptr, MSGTYPE_REQ_RESPONSE);
	if(rc<0)
	{
	    return -1;
	}
	
	if(head_ptr->data_len-sizeof(ST_MSG_COMMON)<=0)//!< 缺失msg数据,返回错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s],  lack of bu connect information", head_ptr->msgid, info_ptr->group_no);
		return -1;
	}
	
	//一般不会返回NULL
	//CServerContext *context_ptr=CServerContext::get_instance();
	CBuGroupInfo *group_info_ptr=NULL;//context_ptr->find_groupinfo(conn_ptr->group_no);
	if(NULL!=group_info_ptr)//也可以直接添加新的group no
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s] unsupport this group no", head_ptr->msgid, info_ptr->group_no);
		return -1;
	}
	
	//ACK消息不用处理,直接返回即可
	if(msglink_check_ccflag((unsigned char *)msg_buff_ptr, CC_ACK_FLAG))
	{
	    return 0;
	}
	
	//todo 将数据打包成外发数据包,并放入到外发数据队列
	//int nsize=head_ptr->data_len-sizeof(ST_MSG_COMMON);
	//unsigned char *ptr=data_ptr;
	INFO_MSG("todo 处理业务请求对应的应答数据...");
	
	
	if(!msglink_check_ccflag((unsigned char *)msg_buff_ptr, CC_NEXT_FLAG))//当前业务已经处理完成,可以处理下一个业务
	{
	    m_link_stat=LNK_STAT_IDLE;//服务注册完毕,可以处理业务
	    return 0;
	}
}

//@brief 控制中心处理业务进程发送的数据;
// 业务进程处理业务请求后,将处理结果返回;
//
int CBuLinkThread::process_butransfer(ST_MSGLINK_BUFF * msg_buff_ptr)
{
	//int err_code=0;
	ST_MSG_HEAD   *head_ptr=&msg_buff_ptr->head;//!< 报文头
	ST_MSG_COMMON *info_ptr=&msg_buff_ptr->commoninfo;//!< msg公共消息
	//unsigned char * data_ptr=msg_buff_ptr->data_buff;//!< msg数据指针
	
	//校验是否是预期消息
	int rc=check_msg_info(msg_buff_ptr, MSGTYPE_REQ_TRANSFER);
	if(rc<0)
	{
	    return -1;
	}
	
	//CBuGroupInfo *bugroupinfo_ptr=CFuncRegister::find_groupinfo((char *)info_ptr->group_no);
	if(head_ptr->data_len-sizeof(ST_MSG_COMMON)<=0)//!< 缺失msg数据,返回错误
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s], lack of bu connect information", head_ptr->msgid, info_ptr->group_no);
		return -1;
	}
	
	//一般不会返回NULL
	//CServerContext *context_ptr=CServerContext::get_instance();
	CBuGroupInfo *group_info_ptr=NULL;//context_ptr->find_groupinfo((char *)info_ptr->group_no);
	if(NULL!=group_info_ptr)//也可以直接添加新的group no
	{
	    ERROR_MSG("msgid=[0x%02x], group_no=[%s],  unsupport this group no", head_ptr->msgid, info_ptr->group_no);
		return -1;
	}
	
	//ACK消息不用处理,直接返回即可
	if(msglink_check_ccflag((unsigned char *)msg_buff_ptr, CC_ACK_FLAG))
	{
	    return 0;
	}
	
	//todo 将数据打包成外发数据包,并放入到外发数据队列
	//int nsize=head_ptr->data_len-sizeof(ST_MSG_COMMON);
	//unsigned char *ptr=data_ptr;
	INFO_MSG("todo 处理业务请求对应的应答数据...");
	
	
	if(!msglink_check_ccflag((unsigned char *)msg_buff_ptr, CC_NEXT_FLAG))//当前业务已经处理完成,可以处理下一个业务
	{
	    m_link_stat=LNK_STAT_IDLE;//服务注册完毕,可以处理业务
	    return 0;
	}
	
	return 0;
}

//@brief [数据传输]控制中心处理业务进程的推送数据;
//注: 业务进程处理业务请求后,将处理结果返回
int CBuLinkThread::process_bupush(ST_MSGLINK_BUFF * msg_buff_ptr)
{
    return -1;
}
