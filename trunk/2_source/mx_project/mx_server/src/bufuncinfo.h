/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义业务功能信息; 包括业务功能信息、业务功能统计信息;
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

#include <time.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
//业务功能定义
//业务功能项
typedef struct __st_bufunc_desc
{
    char  func_id[16];//!< 业务功能id,   字符串格式
	//unsigned int func_no;//!< 业务功能no, 数字格式
	char  func_name[64];//!< 功能名
	char  func_autor[32];//!< 开发人员
    char  func_desc[64];//!< 业务功能说明
	//char func_prog_name[64];//!< 业务程序名
	unsigned int priority;//!< 优先级别; 1~999; 优先级逐级增高
	char  func_comm_type;//!< 通信类型;
}ST_BUFUNC_DESC;

//业务功能统计信息
typedef struct __st_bufunc_static
{
    unsigned int count;        //!< 请求计数
	unsigned int fail_count;   //!< 失败计数
	unsigned int refuse_count; //!< 拒绝请求计数
	unsigned int timeout_count;//!< 超时计数
	time_t   tm_register;   //!< 服务注册时间
	time_t   tm_first_req;  //!< 第一次请求时间;
	time_t   tm_last_req;   //!< 上次请求时间
	
    unsigned int elap_time_total;//!< 该业务处理总时间,单位:毫秒
    unsigned int elap_time_max;  //!< 最大处理时间,单位:微秒
	unsigned int elap_time_min;  //!< 最小处理时间,单位:微秒
	char         if_forbid;      //!< 禁用标记: '1'-停止; '0'-未停止;
}ST_BUFUNC_STATIC;


//业务功能信息: 业务信息+统计信息
typedef struct __st_bufunc_info
{
    ST_BUFUNC_DESC   func_info;  //!< 业务功能信息
	ST_BUFUNC_STATIC func_static;//!< 业务统计信息
}ST_BUFUNC_INFO;

typedef std::vector<ST_BUFUNC_INFO> TBuFuncList;//该类描述一个业务进程支持的业务功能列表

//该类描述一个下级进程支持的业务列表
//class CBuFuncList
//{
//  public:
//    CBuFuncList();
//    virtual ~CBuFuncList();
//  public:
//    void clear();
//    int add_func_info(char *func_id, char *func_desc);
//    int remove_func_info(char *func_id);
//    const ST_BUFUNC_ITEM *find_func_info(char *func_id);
//    
//    const TBuFuncList & get_func_list() { return m_func_vec;}
//    
//    
//  private:
//    std::vector<ST_BUFUNC_ITEM> m_func_vec;//!< 下级系统可能支持多个业务
//    //std::map<int, int> m_func_map;//!< 业务map; key-业务功能id; key-该业务在m_func_vec中索引;
//};
////////////////////////////////////////////////////////////////////////////////////////////////////////
//业务组信息定义
#define MXX_BUGROUPID_INVALID (-1) //!< 无效组id
//业务组信息
typedef struct __st_bugroup_item
{
    unsigned int  group_id;      //!< 组id,控制中心内部使用
	unsigned char group_no[16];  //!< 业务组号, 业务进行分类,由业务进程连接时上传
    unsigned char group_name[32];//!< 组名, 由业务进程连接时上传
	char          prog_name[64]; //!< 业务程序名,由业务进程连接时上传
}ST_BUGROUP_ITEM;

//业务组统计
//typedef __st_bugroup_static
//{
//    time_t        grp_start_time; //!< 业务组注册时间
//	unsigned int  grp_req_count;  //!< 业务组请求计数
//	unsigned int  grp_fail_count; //!< 业务组失败请求计数
//}ST_BUGROUP_STATIC;
////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
