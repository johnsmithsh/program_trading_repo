#ifndef __MTRX_SOCKET_CONN_POOL_H__
#define __MTRX_SOCKET_CONN_POOL_H__


#include <map>

#include "SocketConnInfo.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//定义socket连接池,维护多个socket连接,暂时为长连接设计(短连接是否有必要???)

class CSocketConnPool //CSocketConnPool
{
   private:
      std::map<int, ST_SocketConnInfo> m_net_connection_map;//连接

   private:
      //拷贝构造函数,禁用
      CSocketConnPool(CSocketConnPool &obj);
   public:
      CSocketConnPool();//CSocketConnPool();
      virtual ~CSocketConnPool();
   public:
      ST_SocketConnInfo *create_socketConnInfo(int conn_id,int socket);
      ST_SocketConnInfo *get_socketConnInfo(int conn_id);
      int set_socketConnInfo(int conn_id, ST_SocketConnInfo * conn_info);
      int remove_socketConnInfo(int conn_id);
      int remove_socketConnAll();
};
///////////////////////////////////////////////////////////////////////////////////////////////

#define ConnPool_RCV    1 //接收连接池;
#define ConnPool_SND    2 //发送连接池;
#define ConnPool_Cli    3 //打开的client连接池
#define ConnPool_InOut  4 //输入输出连接池(从同一连接接收数据并返回结果)
//定义socket 连接池管理(单实例模式)
class CSocketConnManage
{
   private:
      CSocketConnManage();
      CSocketConnManage(CSocketConnManage &obj);
   
   public:
      virtual ~CSocketConnManage();

   private:
      CSocketConnPool m_net_recv_conn_pool;//网络连接池
      CSocketConnPool m_net_snd_conn_pool;//发送连接池
      CSocketConnPool m_net_client_pool; //client连接池(当本地socket做客户端时使用)
      CSocketConnPool m_net_inout_pool; 
   public:
      CSocketConnPool * get_net_conn_pool(int pool_id);

   //单实例操作对象与方法;
   private:
      static CSocketConnManage *m_instance;
   public:
      //功能:获取实例指针
      static CSocketConnManage *get_instance();
      //功能:销毁实例
      static int destroy_instance();
};

//功能: 获取连接池
CSocketConnPool * mxx_get_socket_conn_pool(int pool_id)
{
   CSocketConnManage * instance=CSocketConnManage::get_instance();
   if(NULL==instance) return NULL;
   return instance->get_net_conn_pool(pool_id);
}

//功能; 根据sockfd找到对应的连接信息
ST_SocketConnInfo *mxx_get_socket_conn_info(int pool_id, int so)
{
   CSocketConnPool *conn_pool=mxx_get_socket_conn_pool(pool_id);
   if(NULL==conn_pool) return NULL;

   return conn_pool->get_socketConnInfo(so);
}

#endif
