
#include <string.h>
#include <stdio.h>

#include "servercontext.h"
#include "bugroupinfo.h"
#include "bufuncmanage.h"

CServerContext *CServerContext::m_instance=NULL;

CServerContext::CServerContext():m_bcc_id(0),m_max_buno(0)
{
	int num=sizeof(m_bulink_threads)/sizeof(CBuLinkThread);
	char bulinkthreadname[64]={0};
	for(int i=0; i<num; ++i)
	{
		sprintf(bulinkthreadname, "bulinkthread_%03d", i);
		m_bulink_threads[i].set_thread_name(bulinkthreadname);
	}
}

CServerContext::~CServerContext()
{

}

//-----------------------------------------------------------------------------
CBuGroupInfo *CServerContext::find_groupinfo(char *group_no)
{
	if((NULL==group_no)||('\0'==*group_no))
		return NULL;
	return CFuncRegister::find_groupinfo(group_no);
}

void CServerContext::find_groupinfo(unsigned int bu_func_id, std::list<CBuGroupInfo*> &group_list)
{
	group_list.clear();
	if(0==bu_func_id)
		return;
	char func_id_str[16]={0};
	sprintf(func_id_str, "%x", bu_func_id);

	CFuncRegister::find_groupinfo(func_id_str, '\0', group_list);
	return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
int CServerContext::bind_socket_to_bulinkthread(int socket, char *szMsg)
{

	int rc;
	AutoMutex autolock(&m_bulinkthread_mutex);
	//找到一个空闲线程
	CBuLinkThread *bulinkthread_ptr=NULL;
	int num=sizeof(m_bulink_threads)/sizeof(CBuLinkThread);
	for(int i=0; i<num; ++i)
	{
		if(m_bulink_threads[i].check_can_bindsocket())
		{
			bulinkthread_ptr = &m_bulink_threads[i];
			break;
		}
	}

	if(NULL==bulinkthread_ptr)
	{
		if(NULL!=szMsg) sprintf(szMsg, "no available bulinkthread to bind socket");
		return -1;
	}

	rc=bulinkthread_ptr->bind_to_socket(socket);
	if(rc<0)
	{
		if(NULL!=szMsg) sprintf(szMsg, "call bulinkthread::bind_to_socket error![%d]", rc);
		return -2;
	}

	return 0;
}

//@brief 停止bu监听和链接线程
int CServerContext::stop_buthread()
{

	//停止bu监听线程
	m_bulisten_thread.stop_thread();

	//停止bu链接线程
	{
		AutoMutex autolock(&m_bulinkthread_mutex);
		//找到一个空闲线程
		int num=sizeof(m_bulink_threads)/sizeof(CBuLinkThread);
		for(int i=0; i<num; ++i)
		{
			m_bulink_threads[i].stop_thread();
		}
	}

	return 0;
}
