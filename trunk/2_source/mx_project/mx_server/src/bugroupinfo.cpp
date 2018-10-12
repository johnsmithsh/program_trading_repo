
#include <string.h>

#include "bugroupinfo.h"

CBuGroupInfo::CBuGroupInfo()//: m_group_id(MXX_BUGROUPID_INVALID)
{
    m_bufunc_info.clear();
	memset(&m_group_item, 0, sizeof(m_group_item));
	memset(&m_bufunc_info, 0, sizeof(m_bufunc_info));
	m_group_item.group_id=MXX_BUGROUPID_INVALID;
}

CBuGroupInfo::CBuGroupInfo(int group_id)//: m_group_id(group_id)
{
    
    m_bufunc_info.clear();
	memset(&m_group_item, 0, sizeof(m_group_item));
	memset(&m_bufunc_info, 0, sizeof(m_bufunc_info));
	
	if(group_id<=0)
        m_group_item.group_id=MXX_BUGROUPID_INVALID;
	else
	    m_group_item.group_id=group_id;
}

CBuGroupInfo::~CBuGroupInfo()
{
    m_bufunc_info.clear();
	memset(&m_group_item, 0, sizeof(m_group_item));
	memset(&m_bufunc_info, 0, sizeof(m_bufunc_info));
	m_group_item.group_id=MXX_BUGROUPID_INVALID;
}

//@brief 清理数据
void CBuGroupInfo::clear()
{
	m_bufunc_info.clear();
	memset(&m_group_item, 0, sizeof(m_group_item));
	memset(&m_bufunc_info, 0, sizeof(m_bufunc_info));
	m_group_item.group_id=MXX_BUGROUPID_INVALID;
}

//@brief 设置业务组信息; 0-成功;<0-失败;
int CBuGroupInfo::set_group_item(char *group_no, char *group_name, char *prog_name)
{
    if((NULL==group_no)||('\0'==*group_no))
	    return -1;
    memset(m_group_item.group_no, 0, sizeof(m_group_item.group_no));
	strncpy((char *)m_group_item.group_no, group_no, sizeof(m_group_item.group_no)-1);
	
	memset(m_group_item.group_name, 0, sizeof(m_group_item.group_no));
	strncpy((char *)m_group_item.group_name, group_name, sizeof(m_group_item.group_name));
	
	memset(m_group_item.prog_name, 0, sizeof(m_group_item.prog_name));
	strncpy((char *)m_group_item.prog_name, prog_name, sizeof(m_group_item.prog_name)-1);
	
	return 0;	
}
//@brief 设置业务组信息; 0-成功;<0-失败;
int CBuGroupInfo::set_group_item(ST_BUGROUP_ITEM &inGroupItem)
{
    if('\0'==inGroupItem.group_no[0])
	    return -1;
	int group_id=m_group_item.group_id;//group_id不能改变
	
	m_group_item=inGroupItem;
	m_group_item.group_id=group_id;
	
	return 0;
}
//@brief 返回业务组信息;
void CBuGroupInfo::get_group_item(ST_BUGROUP_ITEM &outGroupItem)
{
    outGroupItem=m_group_item;
}

//@brief 注册业务功能,0-成功; <0-失败;
// 如果已经存在,则返回0,但不会覆盖;
int CBuGroupInfo::register_func(int group_id, char *func_id, char *func_desc)
{
    if((MXX_BUGROUPID_INVALID==group_id)||(group_id!=(int)m_group_item.group_id))
	    return -1;
    if((NULL==func_id)||('\0'==*func_id))
	    return -2;
	ST_BUFUNC_DESC func_item;
	memset(&func_item, 0, sizeof(ST_BUFUNC_DESC));
	strncpy(func_item.func_id,  func_id, sizeof(func_item.func_id)-1);
	strncpy(func_item.func_desc, func_desc, sizeof(func_item.func_desc)-1);
	
	return register_func(&func_item);
	
    return -1;
}

//@brief 注册业务功能,0-成功; <0-失败;
// 如果已经存在,则返回0,但不会覆盖;
int CBuGroupInfo::register_func(ST_BUFUNC_DESC *func_item_ptr)
{
    if(NULL==func_item_ptr)
	    return -1;
	if('\0'==func_item_ptr->func_id[0])
	    return -2;
	if(NULL!=get_funcinfo(func_item_ptr->func_id))//业务功能已经存在;
	    return 0;
	
	ST_BUFUNC_INFO func_info;
	memset(&func_info, 0, sizeof(func_info));
	func_info.func_info=*func_item_ptr;
	
	func_info.func_static.if_forbid='0';//默认情况下启用
	
	func_info.func_static.tm_register=time(NULL);
	
	m_bufunc_info.push_back(func_info);//!< 添加到业务功能列表
	
	return 0;
	
}

//@brief 注销业务功能,0-成功; <0-失败;
// 如果已经存在,则返回0,但不会覆盖;
int CBuGroupInfo::unregister_func(int group_id, char *func_id)
{
    if((MXX_BUGROUPID_INVALID==group_id)||(group_id!=(int)m_group_item.group_id))
	    return -1;
    if((NULL==func_id)||('\0'==*func_id))
	    return -2;
    return unregister_func(func_id); 
}

//@brief 注销业务功能,0-成功; <0-失败;
// 如果已经存在,则返回0,但不会覆盖;
int CBuGroupInfo::unregister_func(char *func_id)
{
    if((NULL==func_id)||('\0'==*func_id))
	    return -1;
		
    TBuFuncList::iterator iter;
    for(iter=m_bufunc_info.begin(); iter!=m_bufunc_info.end();)
	{
	    ST_BUFUNC_INFO   &func_info_ref=*iter;
	    if(strcmp(func_id, func_info_ref.func_info.func_id)==0)
		{
		    iter = m_bufunc_info.erase(iter);
		}
		else
		{
		    iter++;
		}
	}
	
	return 0;
}

//@brief 禁用业务功能; 只是禁用,业务功能信息仍然保留
int CBuGroupInfo::forbid_func(char *func_id, bool bForbid/*=false*/)
{
    if((NULL==func_id)||('\0'==*func_id))
	    return -1;
	ST_BUFUNC_INFO *func_info_ptr=get_funcinfo(func_id);
	if(NULL==func_info_ptr)
	    return 0;
	func_info_ptr->func_static.if_forbid=(bForbid) ? '1' : '0';
	return 0;
}

//@brief 根据业务功能id找到对应的业务信息
ST_BUFUNC_INFO * CBuGroupInfo::get_funcinfo(char *func_id)
{
    if((NULL==func_id)||('\0'==*func_id))
	    return NULL;
	TBuFuncList::iterator iter;
    for(iter=m_bufunc_info.begin(); iter!=m_bufunc_info.end();iter++)
	{
	    ST_BUFUNC_INFO   &func_info_ref=*iter;
	    if(strcmp(func_id, func_info_ref.func_info.func_id)==0)
		{
		    return &(*iter);
		}
	}
	return NULL;
}
//@brief 判断该业务组是否支持id对应的业务功能
bool CBuGroupInfo::check_support_func(char *func_id)
{
    return (NULL!=get_funcinfo(func_id)) ? true : false;
}
//@brief 返回所有的业务功能信息
const TBuFuncList &CBuGroupInfo::list_all_func_info()
{
    return m_bufunc_info;
}

////////////////////////////////////////////////////////////////////////////////////////
//业务组列表实现
unsigned int CBuGroupInfoList::m_max_group_id=1024;//group id
CBuGroupInfoList::CBuGroupInfoList()
{
    int n=sizeof(m_group_vec)/sizeof(m_group_vec[0]);
	
	//初始情况下,所有指针都是可用的
	for(int i=0; i<n; i++)
	    m_free_groupinfo.push_back(&m_group_vec[i]);
	m_use_groupinfo.clear();
}

CBuGroupInfoList::~CBuGroupInfoList()
{
    m_free_groupinfo.clear();
	m_use_groupinfo.clear();
}

void CBuGroupInfoList::clear()
{
	int n=sizeof(m_group_vec)/sizeof(m_group_vec[0]);

	m_use_groupinfo.clear();
	//初始情况下,所有指针都是可用的
	for(int i=0; i<n; i++)
	    m_free_groupinfo.push_back(&m_group_vec[i]);

}
//@brief 申请一个可用的业务组内存
CBuGroupInfo *CBuGroupInfoList::alloc()
{
    int group_id = ++m_max_group_id;
	//todo 确定是否加锁
	
	std::list<CBuGroupInfo*>::iterator iter;
	iter=m_free_groupinfo.begin();
	if(iter==m_free_groupinfo.end())
	    return NULL;
	m_free_groupinfo.pop_front();
	
	CBuGroupInfo *group_info_ptr=*iter;
	group_info_ptr->set_groupid(group_id);
	return group_info_ptr;
}
//@brief 释放一个业务组指针
void CBuGroupInfoList::free(CBuGroupInfo *group_info_ptr)
{
    if(NULL==group_info_ptr)
	    return;
	bool exist_flag=false;
	std::list<CBuGroupInfo*>::iterator iter;
	for(iter=m_use_groupinfo.begin(); iter!=m_use_groupinfo.end(); iter++)
	{
	    if(group_info_ptr==*iter)
		{
		    iter=m_use_groupinfo.erase(iter);
			exist_flag=true;
		}
		else
		    ++iter;
	}
	if(exist_flag)
	{
	    group_info_ptr->clear();
		m_free_groupinfo.push_back(*iter);
	}
}

//@brief 查找业务组指针
CBuGroupInfo * CBuGroupInfoList::find_groupinfo(int group_id)
{
    if(group_id<=0)
	    return NULL;
	std::list<CBuGroupInfo*>::iterator iter;
	for(iter=m_use_groupinfo.begin(); iter!=m_use_groupinfo.end(); iter++)
	{
	    CBuGroupInfo* &group_info_ref=*iter;
		if(group_info_ref->get_groupid()==group_id)
		    return *iter;
	}
	return NULL;
}
//@brief 查找业务组指针
CBuGroupInfo *CBuGroupInfoList::find_groupinfo(char * group_no)
{
    if((NULL==group_no)||('\0'==*group_no))
	    return NULL;
    std::list<CBuGroupInfo*>::iterator iter;
	for(iter=m_use_groupinfo.begin(); iter!=m_use_groupinfo.end(); iter++)
	{
	    CBuGroupInfo* &group_info_ref=*iter;
		if(strcmp(group_info_ref->get_groupno(), group_no)==0)
		    return *iter;
	}
	return NULL;
}

//@brief 查找业务功能信息
ST_BUFUNC_INFO *CBuGroupInfoList::find_funcinfo(int group_id, char *func_id)
{
    if(group_id<=0)
	    return NULL;
	if((NULL==func_id)||('\0'==func_id))
	    return NULL;
	CBuGroupInfo* group_info_ptr=this->find_groupinfo(group_id);
	if(NULL==group_info_ptr) return NULL;
	
	return group_info_ptr->get_funcinfo(func_id);
}

//@brief 返回所有业务组信息
int CBuGroupInfoList::get_lists(std::list<CBuGroupInfo*> &outGroupInfo_list)
{
    outGroupInfo_list.clear();
	std::list<CBuGroupInfo*>::iterator iter;
	for(iter=m_use_groupinfo.begin(); iter!=m_use_groupinfo.end(); iter++)
	{
	    outGroupInfo_list.push_back(*iter);
	}
	return 0;
}

//@brief 返回所有业务组信息
std::list<CBuGroupInfo*> & CBuGroupInfoList::get_lists()
{
	return m_use_groupinfo;
}
