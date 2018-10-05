
#include <string.h>
#include <stdint.h>

#include "os_time.h"
//#include "mxx_mem.h"
#include "taskinfo.h"

#define INVALID_SOCKET (-1)
uint64_t CTaskSession::m_current_serial = 0;
uint64_t CTaskSession::next_task_serial()
{
	//todo 后续需要改为原子操作
	return ++m_current_serial;
}

CTaskSession::CTaskSession():m_recv_so(INVALID_SOCKET), m_nLocalRequestId(0)
{
	m_reqFrameVec.clear();
	m_rspFrameVec.clear();
	memset(&m_taskTimeStamp, 0, sizeof(m_taskTimeStamp));
}

CTaskSession::~CTaskSession()
{
	m_recv_so = INVALID_SOCKET;
	m_nLocalRequestId = 0;
	mxx_varmem_t* mem_ptr=NULL;
	//释放缓存
	int count = m_reqFrameVec.size();
	for(int i=0; i<count; ++i)
	{
		//mxx_varmem_destroy((mxx_varmem_t*)(m_reqFrameVec[i]));
		mem_ptr = (mxx_varmem_t*)(m_reqFrameVec[i]);
		mxx_varmem_destroy(mem_ptr);
		m_reqFrameVec[i] = NULL;
	}
	m_reqFrameVec.clear();

	//释放缓存
	count = m_rspFrameVec.size();
	for(int i=0; i<count; ++i)
	{
		//mxx_varmem_destroy((mxx_varmem_t*)(m_rspFrameVec[i]));
		mem_ptr = (mxx_varmem_t*)(m_rspFrameVec[i]);
		mxx_varmem_destroy(mem_ptr);
		m_rspFrameVec[i] = NULL;
	}
	m_rspFrameVec.clear();

	mem_ptr = NULL;

	memset(&m_taskTimeStamp, 0, sizeof(m_taskTimeStamp));
}

//@brief 清空数据
void CTaskSession::clear()
{
	m_recv_so = INVALID_SOCKET;
	m_nLocalRequestId = 0;
	mxx_varmem_t* mem_ptr=NULL;
	//释放缓存
	int count = m_reqFrameVec.size();
	for(int i=0; i<count; ++i)
	{
		//mxx_varmem_destroy((mxx_varmem_t*)(m_reqFrameVec[i]));
		mem_ptr = (mxx_varmem_t*)(m_reqFrameVec[i]);
		mxx_varmem_destroy(mem_ptr);
		m_reqFrameVec[i] = NULL;
	}
	m_reqFrameVec.clear();

	//释放缓存
	count = m_rspFrameVec.size();
	for(int i=0; i<count; ++i)
	{
		//mxx_varmem_destroy((mxx_varmem_t*)(m_rspFrameVec[i]));
		mem_ptr = (mxx_varmem_t*)(m_rspFrameVec[i]);
		mxx_varmem_destroy(mem_ptr);
		m_rspFrameVec[i] = NULL;
	}
	m_rspFrameVec.clear();

	mem_ptr = NULL;

	memset(&m_taskTimeStamp, 0, sizeof(m_taskTimeStamp));
}

//@brief 添加一个请求数据包
void CTaskSession::push_req_frame(mxx_varmem_t *frame)
{
    if(NULL==frame) return;
    if(m_reqFrameVec.empty())
    {
        update_tiemstamp(TIMESTAMP_RECV);
    }
    m_reqFrameVec.push_back(frame);
}

//@brief 添加一个应答数据包
void CTaskSession::push_rsp_frame(mxx_varmem_t *frame)
{
	if(NULL==frame) return;
	m_rspFrameVec.push_back(frame);
}

//@brief 更新任务时间; tiemstamp_type定义见TIMESTAMP_XXX
void CTaskSession::update_tiemstamp(char timestamp_type)
{
	struct timespec ts = {0,0};
	os_gettimeofday(&ts);

	switch(timestamp_type)
	{
	case TIMESTAMP_RECV:
		    m_taskTimeStamp.tm_recv    = ts;
		    break;
	case TIMESTAMP_ASSIGN:
			m_taskTimeStamp.tm_buassign = ts;
			break;
	case TIMESTAMP_DONE:
			m_taskTimeStamp.tm_budone   = ts;
			break;
	case TIMESTAMP_SEND:
	   	m_taskTimeStamp.tm_send     = ts;
			break;
	}
}
