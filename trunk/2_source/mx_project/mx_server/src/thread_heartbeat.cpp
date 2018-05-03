#include <string.h>


#include "ConfigFile.h"
#include "logfile.h"
#include "os_thread.h"

#include "thread_heartbeat.h"

HeartBeatThread::HeartBeatThread(const char *thread_name/*="heart_beat_thread"*/):Thread_Base(thread_name)
{
    memset(&m_beat_item, 0, sizeof(m_beat_item));
	m_enable_flag=HEARTBEAT_DISABLE;//!< 服务启用标记
	m_b_running=false;  //!< 服务运行状态
	m_stop_flag=HEARTBEAT_STOP_FALSE;//!< 停止命令设置标记, 默认没有调用停止命令terminate_service 
}

HeartBeatThread::~HeartBeatThread()
{
    terminate_service();
}

int HeartBeatThread::init()
{
    return 0; //return this->load_ini();
}

//@brief 清理资源
int HeartBeatThread::clear()
{ 
    return -1; 
}

int HeartBeatThread::load_ini(const char *cfgfile)
{
    int rc;
    ConfigFile ini_cfg;
	rc=ini_cfg.load_cfg_file(cfgfile);
	if(rc<0)
	{
	    FATAL_MSG("心跳线程加载配置文件[%s]失败! rc=[%d]", cfgfile, rc);
	    return -1;
	}
	
	char hb_section[]="HEARTBEAT";
	m_enable_flag           =ini_cfg.read_int(hb_section, "enable", HEARTBEAT_DISABLE);  //!< 心跳启用标记
	m_beat_item.beat_timeval=ini_cfg.read_int(hb_section, "timeval", HEARTBEAT_TIMEVAL); //!< 心跳间隔
	memset(m_to_ip, 0, sizeof(m_to_ip));
	ini_cfg.read_string(hb_section, "ip", m_to_ip, sizeof(m_to_ip)-1, "");//!< 心跳数据发送ip地址
	m_to_port               = ini_cfg.read_int(hb_section, "port", -1); //!< 心跳数据发送port端口
	
	INFO_MSG("[%s]%s=%d", hb_section, "enable", m_enable_flag);
	INFO_MSG("[%s]%s=%d", hb_section, "timeval", m_beat_item.beat_timeval);
	INFO_MSG("[%s]%s=%s", hb_section, "ip",  m_to_ip);
	INFO_MSG("[%s]%s=%d", hb_section, "port",  m_to_port);
	
	if(HEARTBEAT_ENABLE==m_enable_flag)
	{
	    if(strlen(m_to_ip)<=0)
		{
		    FATAL_MSG("心跳配置错误[%s]%s不合法!", hb_section, "ip");
			return -3;
		}
		
		if(m_to_port<=0)
		{
		    FATAL_MSG("心跳配置错误[%s]%s不合法!", hb_section, "port");
			return -4;
		}
		
		if(m_beat_item.beat_timeval<0)
		{
		    FATAL_MSG("心跳配置错误[%s]%s不合法!", hb_section, "timeval");
			return -5;
		}
	}
	
	return 0;
}

//
void HeartBeatThread::run()
{
    m_b_running=false;
    if(HEARTBEAT_ENABLE!=m_enable_flag)
	{
	    WARN_MSG("心跳线程禁用");
		return;
	}
	
	while(HEARTBEAT_STOP_TRUE!=m_stop_flag)
	{
	    m_b_running=true;
	    printf("todo send heart beat msg...\n");
		os_thread_msleep(m_beat_item.beat_timeval);		
	}
	
	m_b_running=false;
}

//@brief 停止服务
int HeartBeatThread::terminate_service()
{
    m_stop_flag=HEARTBEAT_STOP_TRUE;
	return 0;
}