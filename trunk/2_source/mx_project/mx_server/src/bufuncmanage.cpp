/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    记录每个下级系统支持的业务功能
 *@details 
 *Others:  
 *Function List:  
         1.………… 
         2.………… 
 *@note  
 *@history:  
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/

#include <list>

#include "bufuncmanage.h"

ItcMutex CFuncRegister::m_mutex=ItcMutex();

 //@brief 初始化业务组信息
int CFuncRegister::reginfo_init()
{
    AutoMutex automutex(&m_mutex);
	CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return -1;
	bugroup_instance->clear();//!< 	清空数据;
	return 0;
}

/**@brief 增加业务组信息,返回group_id; <0-失败;
 * @param
 *    [in]group_item_ptr: 主信息描述(不包括业务功能信息)
 * @retval
 *    0-成功;<0-失败;
**/
int CFuncRegister::reginfo_add_group_info(ST_BUGROUP_ITEM *group_item_ptr)
{
    if(NULL==group_item_ptr)
	    return -1;

	if('\0'==group_item_ptr->group_no[0]) //group_no无效
        return -2;
		
	AutoMutex automutex(&m_mutex);//加锁
	CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return -2;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo((char *)group_item_ptr->group_no);
	if(NULL!=group_info_ptr)
	{
	    return group_info_ptr->get_groupid();
	}
	
	group_info_ptr=bugroup_instance->alloc();//!< 分配一个新的组信息
	if(NULL==group_info_ptr)
	    return -3;
	group_info_ptr->set_group_item((char *)group_item_ptr->group_no, (char *)group_item_ptr->group_name, (char *)group_item_ptr->prog_name);
	return 0;
}

/**@brief 删除业务组信息
 * @param
 *    [in]group_id:业务组id
 * @retval
 *    0-成功;<0-失败;
**/
int CFuncRegister::reginfo_del_group_info(int group_id)
{
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return 0;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo(group_id);
	if(NULL==group_info_ptr)
	    return 0;
	group_info_ptr->clear();
	bugroup_instance->free(group_info_ptr);
	return 0;
}

/**@brief 删除业务组信息
 * @param
 *    [in]group_no:业务组no
 * @retval
 *    0-成功;<0-失败;
**/
int CFuncRegister::reginfo_del_group_info(char *group_no)
{
    if((NULL==group_no)||('\0'==group_no))
	    return 0;
	
	AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return 0;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo(group_no);
	if(NULL==group_info_ptr)
	    return 0;
	group_info_ptr->clear();
	bugroup_instance->free(group_info_ptr);
	return 0;
}

//@brief 注册业务号
int CFuncRegister::reginfo_register_func(int group_id, ST_BUFUNC_DESC *func_item_ptr)
{
    if(NULL==func_item_ptr)
	    return -1;
	if('\0'==func_item_ptr->func_id[0])
	    return -2;
	
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return -4;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo(group_id);
	if(NULL==group_info_ptr)
	    return -5;
	
	int rc=group_info_ptr->register_func(func_item_ptr);
	if(rc<0)
	{
	    return -6;
	}
	
	return 0;
}

//@brief 注销业务功能
int CFuncRegister::reginfo_unregister_func(int group_id, char * func_id)
{
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return -4;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo(group_id);
	if(NULL==group_info_ptr)
	    return -5;
	int rc=group_info_ptr->unregister_func(func_id);
	if(rc<0)
	    return -6;
	return 0;
}

//@brief 停止业务功能
int CFuncRegister::reginfo_stop_func(int group_id, char * func_id)
{
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return -4;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo(group_id);
	if(NULL==group_info_ptr)
	    return -5;
	int rc=group_info_ptr->forbid_func(func_id);
	if(rc<0)
	    return -6;
	return 0;
}

//@brief 查找支持业务功能func_id的所有group_id
int CFuncRegister::find_group_id(char * func_id, char if_forbid, int *pBuGroup, size_t num)
{
    if((NULL==func_id)||('\0'==*func_id))
	    return -1;
	
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return -4;
	
	std::list<CBuGroupInfo *> group_lst;
	bugroup_instance->get_lists(group_lst);
	
	std::list<CBuGroupInfo *>::iterator iter;
	ST_BUFUNC_INFO *bufuncinfo_ptr=NULL;
	unsigned int count=0;
	for(iter=group_lst.begin(); iter!=group_lst.end(); iter++)
	{
	    CBuGroupInfo * &group_info_ptr=*iter;
	    bufuncinfo_ptr=group_info_ptr->get_funcinfo(func_id);
		if(NULL==bufuncinfo_ptr)
		    continue;
		if(('\0'!=if_forbid) && (if_forbid!=bufuncinfo_ptr->func_static.if_forbid))//如果输入禁止标记,则判断该标记
		    continue;
		
		*(pBuGroup+count)=group_info_ptr->get_groupid();
		++count;
		
		if(count>=num)//即将越界
		    break;
	}
	
	return count;	
}

//@brief 找业务组信息,根据业务功能id查找
 void CFuncRegister::find_groupinfo(char * func_id, char if_forbid, std::list<CBuGroupInfo *> &goupinfo_list)
 {
	 goupinfo_list.clear();
	 if((NULL==func_id)||('\0'==*func_id))
		 return;

	 AutoMutex automutex(&m_mutex);//加锁
	 CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	 if(NULL==bugroup_instance)
	 	    return ;

	 std::list<CBuGroupInfo *> group_lst;
	 bugroup_instance->get_lists(group_lst);

	 std::list<CBuGroupInfo *>::iterator iter;
	 ST_BUFUNC_INFO *bufuncinfo_ptr=NULL;
	 unsigned int count=0;
	 for(iter=group_lst.begin(); iter!=group_lst.end(); iter++)
	 {

	    CBuGroupInfo * &group_info_ptr=*iter;
	    bufuncinfo_ptr=group_info_ptr->get_funcinfo(func_id);
	    if(NULL==bufuncinfo_ptr)
	    	continue;
	 	 if(('\0'!=if_forbid) && (if_forbid!=bufuncinfo_ptr->func_static.if_forbid))//如果输入禁止标记,则判断该标记
	 	   continue;
	 	 goupinfo_list.push_back(*iter);
	 }

	 return;
 }

//@brief 找到group_id的业务组信息
CBuGroupInfo  *CFuncRegister::find_groupinfo(unsigned int group_id)
{
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return NULL;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo(group_id);
	return group_info_ptr;
}

//@brief 找到group_id的业务组信息
CBuGroupInfo  *CFuncRegister::find_groupinfo(const char *group_no)
{
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return NULL;
	CBuGroupInfo *group_info_ptr=bugroup_instance->find_groupinfo((char *)group_no);
	return group_info_ptr;
}

//@brief 找到group_id+func_id的业务功能信息
ST_BUFUNC_INFO *CFuncRegister::find_funcinfo(unsigned int group_id, char * func_id)
{
    AutoMutex automutex(&m_mutex);//加锁
    CBuGroupInfoList *bugroup_instance = CBuGroupInfoList::get_instance();
	if(NULL==bugroup_instance)
	    return NULL;
	return bugroup_instance->find_funcinfo(group_id, func_id);
}
