
#include <string.h>
#include <stdio.h>

#include "servercontext.h"
#include "bugroupinfo.h"
#include "bufuncmanage.h"

CServerContext *CServerContext::m_instance=NULL;

CServerContext::CServerContext():m_bcc_id(0),m_max_buno(0)
{

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
