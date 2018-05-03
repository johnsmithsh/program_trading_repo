/*
 * 说明:
 *    实现socket连接信息管理
 *
 */

#include "SocketConnPool.h"

#include <string.h>

CSocketConnPool::CSocketConnPool()
{
}

//析构函数
CSocketConnPool::~CSocketConnPool()
{
    //for conn_info in m_net_connection_map
    //  关闭tcp连接和udp连接的服务端;

    m_net_connection_map.clear();
}

/**
 * 功能: 创建一个socket连接对象
 * 参数:
 *   [in] conn_id: socket连接唯一标记
 *   [in] socket: socket连接描述符,仅对tcp有效(udp只有一个server的socket,没有连接socket);
 * 返回值
 *    socket连接对象指针; NULL-连接失败;
 **/
ST_SocketConnInfo *CSocketConnPool::create_socketConnInfo(int conn_id, int socket)
{
    std::map<int, ST_SocketConnInfo>::iterator iter;
    iter=m_net_connection_map.find(conn_id);
    //连接已经存在,直接返回即可
    if(m_net_connection_map.end()!=iter)
    {
       return &(iter->second);
    }

    ST_SocketConnInfo socket_conn_info;
    memset(&socket_conn_info, 0, sizeof(socket_conn_info));
    m_net_connection_map[conn_id]=socket_conn_info;
    return &(m_net_connection_map.find(conn_id)->second);
}

/**
 * 功能: 创建一个socket连接对象
 * 参数:
 *   [in] conn_id: socket连接唯一标记
 *   [in] socket: socket连接描述符
 * 返回值
 *    socket连接对象指针; NULL-连接失败;
 **/
ST_SocketConnInfo *CSocketConnPool::get_socketConnInfo(int conn_id)
{
   std::map<int, ST_SocketConnInfo>::iterator iter;
   iter=m_net_connection_map.find(conn_id);
   if(m_net_connection_map.end()==iter)//连接不存在
       return NULL;
   return &(iter->second);
}

//功能: 移除socket连接对象
int CSocketConnPool::remove_socketConnInfo(int conn_id)
{
   std::map<int, ST_SocketConnInfo>::iterator iter;
   iter=m_net_connection_map.find(conn_id);
   if(m_net_connection_map.end()==iter)//连接不存在
      return 0;
   m_net_connection_map.erase(iter);
   return 0;
}

//功能: 移除所有的socket连接
int CSocketConnPool::remove_socketConnAll()
{
  //是否需要断开socket连接??

  m_net_connection_map.clear();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
CSocketConnManage* CSocketConnManage::m_instance=NULL;

//功能: 获取实例
CSocketConnManage *CSocketConnManage::get_instance()
{
   if(NULL==m_instance)
   {
       //多线程环境中,此处最好用double check;
       //即加锁->再次判断NULL->创建新实例->解锁;
       m_instance = new CSocketConnManage();
   }
   return m_instance;
}

//功能: 销毁实例
int CSocketConnManage::destroy_instance()
{
   if(NULL!=m_instance)
   {
      delete m_instance;
      m_instance=NULL;
   }
   return 0;
}

//构造函数
CSocketConnManage::CSocketConnManage()
{}

//析构函数
CSocketConnManage::~CSocketConnManage()
{
}

//
CSocketConnPool * CSocketConnManage::get_net_conn_pool(int pool_id)
{
   if( ConnPool_RCV==pool_id)
       return &m_net_recv_conn_pool;
   else if( ConnPool_SND==pool_id)
       return &m_net_snd_conn_pool;
   else if(ConnPool_Cli==pool_id)
       return &m_net_client_pool;
   else if(ConnPool_InOut)
       return &m_net_inout_pool;
   return NULL;
}

