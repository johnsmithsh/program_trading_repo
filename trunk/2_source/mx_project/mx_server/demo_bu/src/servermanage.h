#ifndef _MXX_SERVER_MANAGE_H_
#define _MXX_SERVER_MANAGE_H_

class CServerManage
{
public:
    CServerManage();
    virtual ~CServerManage();
public:
   int init();
    //启动服务
    int start_service();
    //停止服务
    int stop_service();
private:
    CServerManage(const CServerManage &obj);//禁止拷贝构造
    CServerManage & operator=(const CServerManage &a);//重载赋值运算符: 禁用赋值
};

#endif
