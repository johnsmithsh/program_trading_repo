#ifndef _MXX_SERVER_MANAGE_H_
#define _MXX_SERVER_MANAGE_H_

#include "thread_heartbeat.h"
class CServerManage
{
public:
    CServerManage();
    virtual ~CServerManage();
public:
    //启动服务
    int start_service();
    //停止服务
    int stop_service();
private:
    //@brief 启动心跳线程; 0-成功;<0-失败;
    int start_heartbeat_thread(const char *cfgfile);
    //@brief 停止心跳线程; 0-成功;<0-失败;
    int stop_heartbeat_thread();
private:
    CServerManage(const CServerManage &obj);//禁止拷贝构造
    CServerManage & operator=(const CServerManage &a);//重载赋值运算符: 禁用赋值
private:
    HeartBeatThread *m_heart_thread;//!< 心跳线程
    //todo HealthCheckThread
};

#endif
