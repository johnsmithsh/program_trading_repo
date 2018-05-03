
#include <string.h>

#include "bugroupinfo.h"

CBuGroupInfo::CBuGroupInfo(): m_group_id(MXX_BUGROUPID_INVALID)
{
    m_bufunc_info.clear();
}

CBuGroupInfo::CBuGroupInfo(int group_id): m_group_id(group_id)
{
    m_group_id=MXX_BUGROUPID_INVALID;
    m_bufunc_info.clear();
}

CBuGroupInfo::~CBuGroupInfo()
{
    m_group_id=MXX_BUGROUPID_INVALID;
    m_bufunc_info.clear();
}

int CBuGroupInfo::register_func(int group_id, char *func_id, char *func_desc)
{
    if((MXX_BUGROUPID_INVALID==group_id)||(group_id!=m_group_id))
	    return -1;
    if(strlen(func_id)<=0)
	    return -2;
    int rc=m_bufunc_info.add_func_info(func_id, func_desc);
	if(rc<0)
	    return rc;
    return 0;
}

int CBuGroupInfo::unregister_func(int group_id, char *func_id)
{
    if((MXX_BUGROUPID_INVALID==group_id)||(group_id!=m_group_id))
	    return -1;
    if(strlen(func_id)<=0)
	    return -2;
    int rc=m_bufunc_info.remove_func_info(func_id);
	if(rc<0)
	    return rc;
    return 0;   
}

bool CBuGroupInfo::is_support_func(int group_id, char *func_id)
{
    if((MXX_BUGROUPID_INVALID==group_id)||(group_id!=m_group_id))
	    return false;
    if(strlen(func_id)<=0)
	    return false;
    const ST_BUFUNC_ITEM *item_ptr=m_bufunc_info.find_func_info(func_id);
	return (NULL!=item_ptr);
}
