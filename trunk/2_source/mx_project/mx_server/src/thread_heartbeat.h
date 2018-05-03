#ifndef __THREAD_HEARTBEAT_H_
#define __THREAD_HEARTBEAT_H_

#include "os_thread.h"

/*********************************************************************************
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *       心跳线程
                //用于主要说明此程序文件完成的主要功能 
                //与其他模块或函数的接口、输出值、取值范围、 
                    //含义及参数间的控制、顺序、独立及依赖关系
 *@details 
 *Others:  //其他内容说明 
 *Function List:  //主要函数列表，每条记录应包含函数名及功能简要说明
         1.………… 
         2.………… 
 *@note  //注意事项
 *  1. HeartBeatThread析构函数前,一定通过Thread_Base::stop_thread()或HeartBeatThread::terminate_service()停止服务;
 *
 *@history:  //修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/
#include "thread_base.h"

//定义心跳相关项
typedef struct __st_heartbeat_item
{
    time_t last_beattime;//!< 上次心跳时间
	int beat_timeval;//!< 心跳间隔时间, 单位:毫秒
}ST_HEARTBEAT_ITEM;

#define HEARTBEAT_ENABLE 1   //!< 启用
#define HEARTBEAT_DISABLE 0  //!< 禁用

//心跳服务停止命令标记
#define HEARTBEAT_STOP_TRUE  1 //!< 停止服务
#define HEARTBEAT_STOP_FALSE 0 //!<

////心跳服务状态
//#define HEARTBEAT_STATUS_RUNNING 0 //!< 心跳服务状态: 运行中
//#define HEARTBEAT_STATUS_RUNNING 1 //!< 心跳服务状态: 

//默认心跳间隔时间,单位:毫秒ms
#define HEARTBEAT_TIMEVAL 5000 //!< 心跳间隔,默认5秒

class HeartBeatThread : public Thread_Base
{
public:
    HeartBeatThread(const char *thread_name="heart_beat_thread");
	
	virtual ~HeartBeatThread();
public: //!< 接口实现
    virtual int init();//!< 初始化函数; 线程启动前设置
    virtual void run();//!< 线程例程函数
    virtual int clear();//!< 清理资源
	
    virtual int terminate_service();//!< 线程退出命令

public:
    //!< 加载配置文件; 0-成功;<0-失败;
    int load_ini(const char *cfgfile);
    //@brief 获取允许标记; true-允许; false-禁用;
    bool enabled() { return (m_enable_flag==HEARTBEAT_ENABLE); }
private:
    ST_HEARTBEAT_ITEM m_beat_item;//!< 心跳配置项
	int m_enable_flag;//!< 服务启用标记; HEARTBEAT_DISABLE-禁用; HEARTBEAT_ENABLE-启用;
	
	bool m_b_running;//!< 表明服务是否在运行; true-在run循环中;false-退出run循环;
	char m_stop_flag;//!< 服务停止命令标记; HEARTBEAT_STOP_TRUE-收到停止命令, 服务需要退出; HEARTBEAT_STOP_FALSE-没有收到停止命令,服务可以继续运行; 
	
	char m_to_ip[16];//心跳报文发送ip
	int  m_to_port;//心跳报文发送端口
	
};

#endif
