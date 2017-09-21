#ifndef __MXX_NET_SOCKET_H_
#define __MXX_NET_SOCKET_H_

/*
 * 说明: 为便于使用,简单封装socket接口;
 *   GPL
 *
 **/


//功能: 创建一个socket端口; 如果指定ip与端口,则socket绑定ip与端口; 否则只是创建,不做绑定;
//      即 完成socket()->bind(如果指定ip与端口); 
//      一般情况下,创建server端指定ip与端口;客户端不需要指定ip与端口;
//返回值: socket描述符; <=0失败;
int mxx_socket_create(char *ip, int port);

//功能:删除(mxx_socket_create)创建的socket描述符; 返回值: 0-成功; <0-失败;
int mxx_socket_delete(int sockfd);

//功能: socket文件描述符作为server启动监听; 注:调用前必须保证sockfd完成,socker()->bind() 和setsockopt
//返回值:
//   0-成功;<0-失败;
int mxx_socket_listen(int sockfd, int max_conn);


/*
 * 功能: 打开一个socket服务并启动; 即socket()->bind()->listen()
 *      注:启动的是tcp监听; 并指定timeout,单位:毫秒
 * 参数:
 *    [in]ip、port;ip可设置为NULL; port必须大于0
 *    [in]max_conn:最大连接数
 *    [in]timeout:超时时间
 * 返回值:
 *     socket文件描述符; <0-失败;
 **/
int mxx_socket_listen(char *ip, int port,int max_conn, int recv_timeout=-1, int send_timeout=-1);

/*
 * 功能: 设置socket的接收超时时间与发送超时时间;
 *     注:对非阻塞模式,该函数没有意义
 * 参数:
 *    [in]so:socket文件描述符
 *    [in] recv_timeout,send_timeout:超时时间; 正数; <=0不会做任何设置;
 * 返回值:
 *     0-成功; <0-失败;
 * 备注:
 *    1. SO_RECVTIMEO/SO_SNDTIMEO不会影响connect超时?; 没有验证;
 *    2. accept设置超时会导致产生的新连接也存在超时时间?; 没有验证;
 **/
int mxx_socket_set_timeout(int so, int recv_timeout, int send_timeout);

//从socket接收tcp数据
//参数:
//   [in]sockfd
//   [in]buffer/buffsize: 缓存指针和缓存大小
//   [in]data_len: 实际接收数据长度
//
int mxx_socket_recv(int sockfd, char *buffer, int buffsize, int *data_len, int recv_timeout/*=0*/);

//使用socket发送数据 tcp
int mxx_socket_send(int sockfd, const char* buffer, int buflen); 

//功能: 连接tcp服务器; udp暂时还不支持
int mxx_socket_connect(char *ip, int port);


// 功能: socket设置非阻塞模式;返回值:<0-失败; 0-成功;
int mxx_socket_set_nonblock(int so);
//功能: 获取socket非阻塞模式标记; 返回值: 1-非阻塞模式;0-阻塞模式; <0-失败
int mxx_socket_get_nonblock(int so);

#endif
