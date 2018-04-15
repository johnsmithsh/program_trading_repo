/********************************************************************************* 
 *Copyright(C),Your Company 
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  //用于主要说明此程序文件完成的主要功能 
                //与其他模块或函数的接口、输出值、取值范围、 
                    //含义及参数间的控制、顺序、独立及依赖关系 
 *@details 
 *Others:  //其他内容说明 
 *Function List:  //主要函数列表，每条记录应包含函数名及功能简要说明 
         1.………… 
         2.………… 
 *@note  //注意事项
 *@history:  //修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介 
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/  
#ifndef __MXX_GLOBAL_CTRL_H_
#define __MXX_GLOBAL_CTRL_H_
#ifdef WIN32
#else
#endif

//宏定义: 定义系统退出控制标记
#define EXITCODE_TRUE  '1'
#define EXITCODE_FALSE '0'
typedef struct __st_pubctrl
{
    pid_t  main_pid;        //!< 主进程id,由创建该数据结构的进程id填写
	time_t main_start_time;//!< 主进程启动时间
	time_t main_stop_time; //!< 主进程结束时间
	time_t last_chg_time;  //!< 该数据结构上次更新时间; 任何进程修改该结构都需要更新该字段;
	
	char exit_code;//!<系统退出控制标记; 系统下所有进程与线程受该标记控制; 0-启动; 1-退出; 见宏EXITCODE_XXXX
}st_shm_pubctrl;

#endif
 