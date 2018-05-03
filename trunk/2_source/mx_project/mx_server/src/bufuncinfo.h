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
#ifndef _MXX_BUFUNC_INFO_H_
#define _MXX_BUFUNC_INFO_H_

#include <vector>

typedef struct __st_bufunc_info_item
{
    char func_id[16];//!< 业务功能id
    char func_desc[128];//!< 业务功能说明
}ST_BUFUNC_ITEM;

typedef std::vector<ST_BUFUNC_ITEM> TBuFuncList;

//该类描述一个下级进程支持的业务列表
class CBuFuncList
{
  public:
    CBuFuncList();
    virtual ~CBuFuncList();
  public:
    void clear();
    int add_func_info(char *func_id, char *func_desc);
    int remove_func_info(char *func_id);
    const ST_BUFUNC_ITEM *find_func_info(char *func_id);
    
    const TBuFuncList & get_func_list() { return m_func_vec;}
    
    
  private:
    std::vector<ST_BUFUNC_ITEM> m_func_vec;//!< 下级系统可能支持多个业务
    //std::map<int, int> m_func_map;//!< 业务map; key-业务功能id; key-该业务在m_func_vec中索引;
};
#endif
