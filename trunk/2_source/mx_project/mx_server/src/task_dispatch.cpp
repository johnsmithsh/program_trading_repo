#include "task_dispatch.h"
#include "servercontext.h"

//@brief 找到可处理业务的业务线程
CBuLinkThread * find_readybuthread()
{
	CServerContext *ctx_instance=CServerContext::get_instance();
	if(NULL==ctx_instance)
		return NULL;

	//找到空闲状态的
	CBuLinkThread *buthread=NULL;
	int count = sizeof(ctx_instance->m_bulink_threads)/sizeof(CBuLinkThread);
	for(int i=0; i<count; ++i)
	{
		buthread = & ctx_instance->m_bulink_threads[i];
		if(LNK_STAT_READY==buthread->get_link_stat())
			return buthread;
	}

	return NULL;
}

//@brief 向业务线程分配任务
int CTaskDispatch::dispatch_task(CTaskSession * task_session)
{
	if(NULL==task_session) return -1;

	CBuLinkThread *buthread=find_readybuthread();
	if(NULL==buthread)//没有可用线程
	{
		//todo 构建应答报文和错误信息,添加到完成队列
		return -2;
	}

	task_session->update_tiemstamp(TIMESTAMP_ASSIGN);//!<设置分配任务线程时间

	//请求分配给指定的业务线程
	int rc=buthread->send_request_to_bu(task_session);
	if(rc<0)
	{
		//todo 构建应答报文和错误信息,添加到完成队列
		return -3;

	}

	m_task_process_que.push_back(task_session);
	return 0;
}

//@brief (业务线程)任务已经完成
int CTaskDispatch::task_done(CTaskSession * task_session)
{
	CTaskSession *pTaskSession=task_session;
	std::list<CTaskSession *>::iterator iter;
	for(iter=m_task_process_que.begin(); iter!=m_task_process_que.end(); ++iter)
	{
		if(*iter==task_session)
		{
			pTaskSession = *iter;
			m_task_process_que.erase(iter);
			break;
		}
	}

	pTaskSession->update_tiemstamp(TIMESTAMP_DONE);//!<设置任务完成时间
	m_task_done_que.push(pTaskSession);
	return 0;
}
