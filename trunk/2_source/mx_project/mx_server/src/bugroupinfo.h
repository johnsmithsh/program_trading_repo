
/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义一组服务,该组服务必须提供相同的功能; 用于描述下级服务组
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
#ifndef _MXX_BU_GROUP_INFO_H_
#define _MXX_BU_GROUP_INFO_H_
#include "bufuncinfo.h"

#define MXX_BUGROUPID_INVALID (-1) //!< 无效组id
//定义一组服务,记录该组服务支持的业务
class CBuGroupInfo
{
  public:
    CBuGroupInfo();
	CBuGroupInfo(int group_id);
	virtual ~CBuGroupInfo();
  public:
    void set_groupid(int group_id) { m_group_id=group_id; }
	int  get_groupid() { return m_group_id; }
    int register_func(int group_id, char *func_id, char *func_desc);
	int unregister_func(int group_id, char *func_id);
	bool is_support_func(int group_id, char *func_id);
	//CBuFuncList &get_func_list();
  private:
    int m_group_id;//!< 服务组号
	
  private:
    CBuFuncList m_bufunc_info;
};
#endif
