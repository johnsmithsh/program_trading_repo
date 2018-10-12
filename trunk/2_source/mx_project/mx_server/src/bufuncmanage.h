/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    该文件实现业务组信息操作,并保证多线程安全.
 *    1. 增加业务组/删除业务组
 *    2. 注册业务功能/注销业务功能/业务功能查找等
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
#ifndef _MXX_BUFUNC_MANAGE_H_
#define _MXX_BUFUNC_MANAGE_H_

#include "bugroupinfo.h"
#include "itc_mutex.h"

//以下函数接口与业务直接相关
//本不打算增加新类了,考虑到并发问题还是增加吧!
//业务功能注册器, 与业务直接相关
//管理业务组+业务功能信息
class CFuncRegister
{
  public:
    //初始化业务组和业务相关信息
    static int reginfo_init();
    //业务组相关接口
	//一般情况下,业务组信息,只会增加不会删除; 否则丢失业务组下面的业务组信息和业务功能统计信息
    //@brief 增加业务组信息,返回group_id; <0-失败;
    static int reginfo_add_group_info(ST_BUGROUP_ITEM *group_item_ptr);
    static int reginfo_del_group_info(int group_id);
    static int reginfo_del_group_info(char *group_no);
    
    //@brief 注册业务号
    static int reginfo_register_func(int group_id, ST_BUFUNC_DESC *func_item_ptr);
    static int reginfo_unregister_func(int group_id, char * func_id);
    static int reginfo_unregister_func(char * func_id);
    //@brief 停止业务服务
    static int reginfo_stop_func(int group_id, char *func_id);
    static int reginfo_stop_func(char * func_id);
    static int reginfo_stop_func();
    
    //@brief 根据业务功能id找到支持该业务的业务组id;
    //  用于查找哪些group支持业务功能;
    static int find_group_id(char * func_id, char if_forbid, int *pBuGroup, size_t num);
    
    //@brief 找业务组信息,根据业务功能id查找
    static void find_groupinfo(char * func_id, char if_forbid, std::list<CBuGroupInfo *> &goupinfo_list);
    //@brief 业务组id找到业务组信息
    static CBuGroupInfo  *find_groupinfo(unsigned int group_id);
    //@brief 业务组no找到业务组信息
    static CBuGroupInfo  *find_groupinfo(const char *group_no);

    //@brief 找到业务描述信息
    static ST_BUFUNC_INFO *find_funcinfo(unsigned int group_id, char * func_id);
  private:
    static ItcMutex m_mutex;//!< 希望这个互斥锁不会影响性能吧!
};

#endif
