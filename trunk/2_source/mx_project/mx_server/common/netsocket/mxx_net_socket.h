#ifndef __MXX_NET_SOCKET_H_
#define __MXX_NET_SOCKET_H_

#include <arpa/inet.h>

//封装socket错误码;
#define MXX_SOCKET_INVSOCK     (-1)     //!< 无效socket描述符
#define MXX_SOCKET_SUCC        (0)      //!< socket操作成功; 
#define MXX_SOCKET_BUFF_ERROR  (-1000)  //!< 缓冲区错误 如缓冲区指针无效 || 缓存大小<=0;
#define MXX_SOCKET_CLOSED      (-1001)  //!< socket关闭
#define MXX_SOCKET_TIMEOUT     (-1002)  //!< 超时
#define MXX_SOCKET_PARAM_ERROR (-1003)  //!< 参数错误
/*
 * 说明: 为便于使用,简单封装socket接口;
 *   GPL
 *
 **/

/**
 * @brief 创建一个socket端口; 默认tcp端口
 * @param
 *
 * @retval socket描述符; <0失败;
 **/
int mxx_socket_create(int domain=AF_INET, int type=SOCK_STREAM);

//功能: 创建一个socket端口; 如果指定ip与端口,则socket绑定ip与端口; 否则只是创建,不做绑定;
//      即 完成socket()->bind(如果指定ip与端口); 
//      一般情况下,创建server端指定ip与端口;客户端不需要指定ip与端口;
//返回值: socket描述符; <0失败;
int mxx_socket_create(char *ip, int port);

/**
 * @brief 绑定socket端口;
 * @param
 *
 * @retval 0-成功; <0失败;
 **/
int mxx_socket_bind(int sockfd, char *ip, int port, int domain=AF_INET);

//@brief 删除(mxx_socket_create)创建的socket描述符; 返回值: 0-成功; <0-失败;
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
//int mxx_socket_listen(char *ip, int port,int max_conn, int recv_timeout=-1, int send_timeout=-1);

//@brief 接收客户端连接; 仅支持tcp
int mxx_socket_accept(int sockfd, int millisecond);
//@brief 接收客户端连接; 仅支持tcp
int mxx_socket_accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen, int millisecond);

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
//int mxx_socket_set_timeout(int so, int recv_timeout, int send_timeout);

//从socket接收tcp数据
//参数:
//   [in]sockfd
//   [in]buffer/buffsize: 缓存指针和缓存大小
//   [in]data_len: 实际接收数据长度
//
int mxx_socket_recv(int sockfd, char *buffer, int buffsize, int *data_len, int recv_timeout/*=0*/);

//使用socket发送数据 tcp
int mxx_socket_send(int sockfd, const char* buffer, int buflen); 

//@brief 连接服务端socket
int mxx_socket_connect(int sockfd, char *ip, int port, int millisecond);
//@brief 连接服务端socket
int mxx_socket_connect(int sockfd, struct sockaddr_in *addr, int addrlen, int millisecond);
//功能: 连接tcp服务器; udp暂时还不支持
int mxx_socket_connect(char *ip, int port);


// 功能: socket设置非阻塞模式;返回值:<0-失败; 0-成功;
//int mxx_socket_set_nonblock(int so);
//功能: 获取socket非阻塞模式标记; 返回值: 1-非阻塞模式;0-阻塞模式; <0-失败
//int mxx_socket_get_nonblock(int so);

/////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------
//socket属性设置
//------------------------------------------------------------------------------------------
//@brief 接收超时时间设置
int mxx_sockopt_set_recvtimeout(int sockfd, int millisecond);
//@brief 发送超时时间设置
int mxx_sockopt_set_sendtimeout(int sockfd, int millisecond);
//@brief 通过SO_SNDTIMEO让conn超时
int mxx_sockopt_set_conntimeout(int sockfd, int millisecond);

/**
 * @brief 设置阻塞模式(默认情况下,是阻塞模式);
 * @param
 *     blockmode: true-阻塞模式; false-非阻塞模式;
 **/
int mxx_sockopt_set_block(int sockfd, bool blockmode);

/**
 * @brief
 * @note
 *      SO_REUSEADDR  ???
 *      SO_DONTLINGER ???
 **/
int mxx_sockopt_set_timewaitmode(int sockfd, int timewait_millisecond);

/**
 * @biref 用于设置socket的close行为
 * @param
 *    [in]sockfd:socket描述符
 *    [in]enable: true-启用linger; false-停用linger;
 *    [in]second: 延迟时间,单位:秒; 范围[0, 10]
 * @note
 *  注意：
 *    这个选项需要谨慎使用，尤其是强制式关闭，会丢失服务器发给客户端的最后一部分数据。
 *    UNP中:
 *       The TIME_WAIT state is our friend and is there to help us(i.e., to let the old duplicate segments expire in the network).
 *    使用该选项,程序一定要检查close返回值; close超时返回， 则close返回-1 && errno=EWOULDBLOCK;
 **/
int mxx_sockopt_set_linger(int sockfd, bool enable, int second);

/**
 * @brief keepalive属性
 * @param
 *    [in]sockfd
 *    [in]enable: true-启用keepalive; false-禁用keepalive;
 *    [in]keepIdle:     单位:秒; <0-使用默认值; 如果在指定秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
 *    [in]keepInterval: 单位:秒; <0-使用默认值; 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
 *    [in]keepCount:    单位:次; <0-使用默认值; 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
 * @note
 *    仅适用于TCP;
 *    大多数内核是基于整个内核维护该时间参数的,而不是基于每个socket维护;
 *    常用于服务器,用于检测半开连接并关闭;
 **/
int mxx_sockopt_set_keepalive_info(int sockfd, bool enable, int keepIdle=-1, int keepInterval=-1, int keepCount=-1);

/**
 * @brief 启用广播模式
 * @param
 **/
//int mxx_sockopt_set_enablebroadcast(int sockfd, int broadcastmode);


//int mxx_sockopt_set_recvbuffsize(int sockfd, size_t recvbuffsize);
//int mxx_sockopt_set_sendbuffsize(int sockfd, size_t sendbuffsize);
//-------------------------------------------------------------------------------------------------------------------
//阻塞发送与接收, 直到处理指定长度数据
/**
 * @brief 从socket接收tcp数据,直到收到制定长度数据或者超时;
 * @param
 *   [in]sockfd
 *   [in]buff/buffsize: 缓存指针和缓存大小
 *   [out]data_len: 实际接收数据长度
 * @retval
 *    0-成功; MXX_SOCKET_BUFF_ERROR-缓冲区错误; MXX_SOCKET_CLOSED-socket关闭; MXX_SOCKET_TIMEOUT-超时; 其他小于0-错误;
 * @note
 *   该函数暂时仅支持tcp;
 **/
int mxx_socket_recv_waitall(int sockfd, char *buff, int buffsize, int *rcv_len, int timeout=0);
/**
 * @brief socket发送tcp数据,直到发送长度数据或者超时;
 * @param
 *   [in]sockfd
 *   [in]buff/buffsize: 缓存指针和缓存大小
 *   [out]snd_len: 实际发送数据长度
 * @retval
 *    0-成功;
 *    其他小于0-错误;
 **/
int mxx_socket_send_waitall(int sockfd, char *buff, int datalen,  int *snd_len, int timeout=0);

//非阻塞发送与接收,立即返回
/**
 * @brief 从socket接收tcp数据,直到收到指定长度数据或者立即返回;
 * @param
 *   [in]sockfd
 *   [in]buff/buffsize: 缓存指针和缓存大小
 *   [out]rcv_len: 实际接收数据长度
 * @retval
 *    0-成功; MXX_SOCKET_BUFF_ERROR-缓冲区错误; MXX_SOCKET_CLOSED-socket关闭; MXX_SOCKET_TIMEOUT-超时; 
 *    其他小于0-错误;
 **/
int mxx_socket_recv_nowait(int sockfd, char *buff, int buffsize, int *rcv_len);
/**
 * @brief socket发送tcp数据,直到发送长度数据或者超时;
 * @param
 *   [in]sockfd
 *   [in]buff/buffsize: 缓存指针和缓存大小
 *   [out]snd_len: 实际发送数据长度
 * @retval
 *    0-成功;
 *    其他小于0-错误;
 **/
int mxx_socket_send_nowait(int sockfd, char *buff, int datalen,  int *snd_len);

int mxx_socket_read(int sockfd, char *buff, int datalen,  int *snd_len, int timeout=0);
int mxx_socket_write(int sockfd, char *buff, int datalen,  int *snd_len, int timeout=0);
#endif
