#include "mxx_net_socket.h"
#include <sys/epoll.h>

//#include <string.h>
//#include <stdlib.h>


#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <errno.h>

#include "os_time.h"

/*
 * 功能: 创建一个socket端口; 如果指定ip与端口,则socket绑定ip与端口; 否则只是创建,不做绑定;
 * 参数:
 *    [in]ip
 *    [in]port
 * 返回值:
 *    socket文件描述符; <=0失败;
 **/
int mxx_socket_create(char *ip, int port)
{
   int so_fd=-1;
   int rc=-1;
   //创建socket
   so_fd=socket(AF_INET, SOCK_STREAM, 0);
   if(so_fd<=0)
      return -1;

   if(port>0)
   {
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family=AF_INET;
      /*
       * INADDR_ANY:任意地址;
       * INADDR_NONE:inet_addr("255.255.255"),广播地址;
       * INADDR_LOOPBACK:inet_addr("127.0.0.1")
       **/
     if((NULL==ip)||(strlen(ip)<=0))
       addr.sin_addr.s_addr=htonl(INADDR_ANY);
     else
       addr.sin_addr.s_addr=inet_addr(ip);
     addr.sin_port=htons(port);
     if((rc=bind(so_fd, (struct sockaddr *)&addr, sizeof(addr)))<0)
     {
        close(so_fd);
        return -2;
     }
      
   }

   //对方主机崩溃
   //(如断开网络服务器、主机崩溃等)
   //为了在不发送数据的情况下检测到对方崩溃
   //建议设置超时时间或使用SO_KEEPALIVE;
   //说明:
   //    如果客户数据没有收到响应,则返回错误ETIMEDOUT
   //    如果中间路由器发现服务器主机不可达,则响应"destination unreachable"ICMP消息,返回错误EHOSTUNREACH或ENETUNREACH;
   //服务器崩溃重启
   //  如果服务器崩溃时客户没有发送数据,客户将不知道服务器出现问题(假设未使用SO_KEEPALIVE);
   //  此时服务器TCp对所有来自客户的数据都使用RST响应;
   //  客户收到RST响应,如果阻塞在read,则会返回ECONNRESET错误;
   //建议:
   //   客户检测服务器可用状态;
   //

   //对方服务器终止
   // 会收到RST信号,此时如果继续写操作,则进程会收到SIGPIPE信号
   //建议:
   //    设置SIGPIPE信号处理函数;
   
   //连接正常终止
   //  发起FIN的一方会处于TIEM_WAIT状态,占用系统资源;
   //建议:
   //  为防止连接处于TIME_WAIT状态,推荐使用SO_REUSEPORT
   return so_fd;
}

/**
 * @brief 创建一个socket端口; 默认tcp端口
 * @param
 *
 * @retval socket描述符; <0失败;
 **/
int mxx_socket_create(int domain/*=AF_INET*/, int type/*=SOCK_STREAM*/)
{
   /*
    * AF_INET, SOCK_STREAM: tcp
	* AF_INET, SOCK_DDGRAM: udp
	*/
   int sockfd=socket(domain, type, 0);
   if(sockfd<0)
      return errno;
   return  sockfd;	  
}

/**
 * @brief 绑定socket端口;
 * @param
 *
 * @retval 0-成功; <0失败;
 **/
int mxx_socket_bind(int sockfd, char *ip, int port, int domain/*=AF_INET*/)
{
    struct sockaddr_in addr;
	int rc;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=domain;
    /*
     * INADDR_ANY:任意地址;
     * INADDR_NONE:inet_addr("255.255.255"),广播地址;
     * INADDR_LOOPBACK:inet_addr("127.0.0.1")
     **/
    if((NULL==ip)||(strlen(ip)<=0))
       addr.sin_addr.s_addr=htonl(INADDR_ANY);
    else
       addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(port);
    if((rc=bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)))<0)
    {
       return errno;
    }
	
	return 0;
}

//@brief 删除创建的socket描述符
int mxx_socket_delete(int sockfd)
{
   return close(sockfd);
}

//功能: socket文件描述符作为server启动监听; 注:调用前必须保证sockfd完成,socker()->bind() 和setsockopt
//返回值:
//   0-成功;<0-失败;
int mxx_socket_listen(int sockfd, int max_conn)
{
   int rc=listen(sockfd,max_conn);
   if(rc<0)
       return errno;
   return 0;
}

/*
 * 功能: 打开一个socket服务并启动;
 *      注:启动的是tcp监听; 并指定timeout,单位:毫秒
 *      完成socket()->bind()->listen()
 * 参数:
 *    [in]ip、port;
 *    [in]max_conn:最大连接数
 *    [in]timeout:超时时间
 * 返回值:
 *     socket文件描述符; <0-失败;
 **/
//int mxx_socket_listen(char *ip, int port,int max_conn, int recv_timeout/*=10*/, int send_timeout/*=10*/)
//{
//   int so_fd;
//   int rc;
//
//   //输入参数校验
//   if(max_conn<=0)//最大连接数必须大于0
//      return -1;
//   if(port<=0)//端口号必须大于0
//      return -1;
//   
//   //创建socket
//   so_fd=socket(AF_INET, SOCK_STREAM, 0);
//   if(so_fd<=0)
//      return -1;
//
//   //bind绑定
//   struct sockaddr_in addr;
//   memset(&addr, 0, sizeof(addr));
//   addr.sin_family=AF_INET;
//   /*
//    * INADDR_ANY:任意地址; 
//    * INADDR_NONE:inet_addr("255.255.255"),广播地址;
//    * INADDR_LOOPBACK:inet_addr("127.0.0.1")
//    * */
//   if((NULL==ip)||(strlen(ip)<=0))
//     addr.sin_addr.s_addr=htonl(INADDR_ANY);
//   else
//     addr.sin_addr.s_addr=inet_addr(ip);
//   addr.sin_port=htons(port);
//   if((rc=bind(so_fd, (struct sockaddr *)&addr, sizeof(addr)))<0)
//   {
//      close(so_fd);
//      return -2;
//   }
//
//   //设置超时时间
//   rc=mxx_socket_set_timeout(so_fd, recv_timeout, send_timeout);
//   if(rc<0)
//   {
//     close(so_fd);
//      return -3;
//   }
//
//   //启动监听
//   rc=listen(so_fd,max_conn);
//   if(rc<0)//启动监听失败
//   {
//     close(so_fd);
//      return -4;
//   }
//
//   
//   return so_fd;
//}


//@brief 接收客户端连接; 仅支持tcp
int mxx_socket_accept(int sockfd, int millisecond)
{
    int so=accept(sockfd,NULL,NULL);
	if(so<0)
	    return errno;
		
	return so;    
}

//@brief 接收客户端连接; 仅支持tcp
int mxx_socket_accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen, int millisecond)
{
    int so=accept(sockfd,(struct sockaddr*)addr,addrlen);
	if(so<0)
	    return errno;
		
	return so;    
}

//获取浮点数的整数部分
static int __mxx_intpart_(double d)
{
   double intpart=0.0;
   double fractpart=0.0;
   fractpart=modf(d, &intpart);
   return (int)intpart;
}

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
//int mxx_socket_set_timeout(int so, int recv_timeout, int send_timeout)
//{
//   struct timeval timeout={0,0};
//   int rc;
//
//   if(recv_timeout>0)
//   {
//      timeout.tv_sec=__mxx_intpart_(recv_timeout/1000);
//      timeout.tv_usec=(recv_timeout%1000)*1000;//微妙
//      rc=setsockopt(so, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
//      if(rc<0)
//         return -1;
//   }
//
//   if(send_timeout>0)
//   {
//      timeout.tv_sec=__mxx_intpart_(send_timeout/1000);
//      timeout.tv_usec=(send_timeout%1000)*1000;//微妙
//      rc=setsockopt(so, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
//      if(rc<0)
//         return -1;
//   }
//
//  return 0;
//   
//}

//从socket接收tcp数据
//参数:
//   [in]sockfd
//   [in]buffer/buffsize: 缓存指针和缓存大小
//   [in]data_len: 实际接收数据长度
//
int mxx_socket_recv(int sockfd, char *buffer, int buffsize, int *data_len, int recv_timeout/*=0*/)
{
   int rc;
   int recvd_len=0;//已接收数据长度
   char *ptr=buffer;
   int err_code=0;
   
   if( (NULL==buffer) || (buffsize<=0) )
      return -1;

   //if(recv_timeout>0)
   //{
   //   mxx_socket_set_timeout(sockfd, recv_timeout, -1);
   //}

   while(recvd_len<buffsize)
   {
       /*
        * 默认socket的recv是阻塞的; 无论阻塞与非阻塞,rc<0表示失败; =0连接关闭; >0-收到数据;
        *     当数据没有到达,默认情况下(阻塞模式)recv会等待数据到达;
        * 注意: rc<0 &&((errno==EINTR)||(errno==EWOULDBLOCK)||(errno==EAGAIN)) 认为是正常的,继续接收数据
        *   当非阻塞模式下,如果socket没有可用数据,则返回-1,并将errno设置为EAGAIN或EWOULDBLOCK
        **/
       rc=recv(sockfd, ptr, buffsize-recvd_len,0);
       if(0==rc)//连接关闭
          break;
       if(rc<0)
       {
	       err_code=errno;
           //非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
           if( (EAGAIN==err_code) || (EWOULDBLOCK==err_code) ) //socket被标记为非阻塞,接收被阻塞或超时
             break;
           else if(EINTR==err_code)//在收到数据前,被信号中断
             continue;
           return err_code;
       }
       recvd_len +=rc;
       *data_len = recvd_len;
   }

   return 0;
}

//使用socket发送数据 tcp
int mxx_socket_send(int sockfd, const char* buffer, int buflen) 
{
  int rc;
  size_t total = buflen;
  const char *p = buffer;
  int retry_count=0;//重试次数
  
  int err_code=0;
  while(1) {
    rc = send(sockfd, p, total, 0);
    if(rc < 0) {
	  err_code=errno;
      // 当send收到信号时,可以继续写,但这里返回-1.
      if(err_code == EINTR)
        return -1;
      // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
      // 在这里做延时后再重试.
      if(err_code == EAGAIN) {
        ++retry_count;
        if(retry_count>3)//重试超过次数,则退出
            break;
        usleep(1000);
        continue;
      }
      return -1;
    }
    if((size_t)rc == total)
      return buflen;
    total -= rc;
    p += rc;
  }
  
  return (buflen-total);
}

//@brief 连接服务端socket
int mxx_socket_connect(int sockfd, char *ip, int port, int millisecond)
{
   struct sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family=AF_INET;
   addr.sin_port=htons(port);
   addr.sin_addr.s_addr = inet_addr(ip);
   
   int rc=mxx_socket_connect(sockfd, &addr, sizeof(addr), millisecond);//connect(sockfd, &addr, sizeof(addr));
   if(rc<0)
   {
        return errno;
   }
   
   return 0;
}

//@brief 连接服务端socket
int mxx_socket_connect(int sockfd, struct sockaddr_in *addr, int addrlen, int millisecond)
{
    int rc=connect(sockfd, (struct sockaddr*)&addr, addrlen);
   if(rc<0)
   {
        return errno;
   }
   
   return 0;
}

//功能: 连接tcp服务器; udp暂时还不支持
int mxx_socket_connect(char *ip, int port)
{
   int rc=0;
   if( (NULL==ip) || ('\0'==*ip) )
       return -1;
   if(port<=0)
       return -2;
   //创建socket
   int sofd=socket(AF_INET,SOCK_STREAM,0);
   if(sofd<0)
       return -3;
   struct sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family=AF_INET;
   addr.sin_port=htons(port);
   addr.sin_addr.s_addr = inet_addr(ip);

   rc=connect(sofd, (struct sockaddr *)&addr, sizeof(addr));
   if(rc<0)
   {
        return -4;
   }

   return sofd;
}


//int mxx_socket_set_nonblock(int sockfd)
//{
//   int flags;
//   flags = fcntl(sockfd, F_GETFL, 0);
//   if(-1==flags)
//      return -1;
//   flags=fcntl(sockfd, flags|O_NONBLOCK);
//   if(-1==flags)
//     return -2;
//   return 0;
//}
//
//int mxx_socket_get_nonblock(int sockfd)
//{
//   int flags;
//   flags = fcntl(sockfd, F_GETFL, 0);
//   if(-1==flags)
//      return -1;
//   return (flags & O_NONBLOCK) ? 1 : 0;
//}

////////////////////////////////////////////////////////////////////////////////////////////

int mxx_sockopt_set_recvtimeout(int sockfd, int millisecond)
{
    if(millisecond<=0) 
	    return MXX_SOCKET_SUCC;
    struct timeval tv_macro;//!< struct timeval精确到微秒
	tv_macro.tv_sec=int(millisecond/1000);   //!< 秒
	tv_macro.tv_usec=(millisecond%1000)*1000;//!< 微秒
	int rc=setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv_macro,sizeof(struct timeval));
	if(rc<0)
	    return errno;
    return MXX_SOCKET_SUCC;    
}

int mxx_sockopt_set_sendtimeout(int sockfd, int millisecond)
{
    if(millisecond<=0)
	    return MXX_SOCKET_SUCC;
    struct timeval tv_macro;//!< struct timeval精确到微秒
	tv_macro.tv_sec=int(millisecond/1000);   //!< 秒
	tv_macro.tv_usec=(millisecond%1000)*1000;//!< 微秒
	int rc=setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv_macro,sizeof(struct timeval));
	if(rc<0)
	    return errno;
    return MXX_SOCKET_SUCC;
}

int mxx_sockopt_set_conntimeout(int sockfd, int millisecond)
{
    return mxx_sockopt_set_sendtimeout(sockfd, millisecond);
}


/**
 * @brief 设置阻塞模式(默认情况下,是阻塞模式);
 * @param
 *     blockmode: true-阻塞模式; false-非阻塞模式;
 **/
int mxx_sockopt_set_block(int sockfd, bool blockmode)
{
    int rc;
    if(!blockmode)//非阻塞
	{
	    rc=fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
	}
	else//阻塞
	{
	    rc=fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) &(~O_NONBLOCK));
	}
	if(rc<0)
	    return errno;//返回错误码
	return MXX_SOCKET_SUCC;
}

/**
 * @brief
 * @note
 *      SO_REUSEADDR  ???
 *      SO_DONTLINGER ???
 * @note
 *   仅适用于tcp/sctp
 **/
int mxx_sockopt_set_timewaitmode(int sockfd, int timewait_millisecond)
{
   return -1;
}

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
int mxx_sockopt_set_linger(int sockfd, bool enable, int second)
{
    
    /*details  SO_LINGER
	 * 1. 仅仅适用于TCP,SCTP;
	 * 2. 在默认情况下,当调用close关闭socke的使用,close会立即返回;
	 *   但是,如果send buffer中还有数据,系统会试着先把send buffer中的数据发送出去,然后close才返回.
	 *   SO_LINGER选项则是用来修改这种默认操作的.
	 * 3.
	 * #include <sys/socket.h>  
     *  struct linger {
     *      int l_onoff  //0=off, nonzero=on(开关)
     *      int l_linger //linger time(延迟时间)
	 *  }
	 *  
	 *  1) 0==l_onoff
     *    当l_onoff被设置为0的时候,将会关闭SO_LINGER选项,即TCP或则SCTP保持默认操作:close立即返回.l_linger值被忽略.
     *
     *  2) 0!=l_lineoff && 0==l_linger
     *    当调用close的时候,TCP连接会立即断开.sendbuffer中未被发送的数据将被丢弃,并向对方发送一个RST信息.
	 *    由于这种方式，是非正常的4中握手方式结束TCP链接，所以，TCP连接将不会进入TIME_WAIT状态，这样会导致新建立的可能和就连接的数据造成混乱;
	 *  3) 0!=l_lineoff && 0!=l_linger
	 *     在这种情况下,close返回得到延迟.
	 *     调用close去关闭socket的时候,内核将会延迟.即,如果send buffer中还有数据尚未发送,该进程将会被休眠直到一下任何一种情况发生:
     *       *send buffer中的所有数据都被发送并且得到对方TCP的应答消息(这种应答并不是意味着对方应用程序已经接收到数据，在后面shutdown将会具体讲道);
     *       *延迟时间消耗完. 在延迟时间被消耗完之后,send buffer中的所有数据都将会被丢弃;
     *     另:两种情况中,如果socket被设置为O_NONBLOCK状态,程序将不会等待close返回, send buffer中的所有数据都将会被丢弃.
	 *        所以,需要判断close的返回值.在send buffer所有数据都被发送之前并且延迟时间没有消耗完,close返回,则close将会返回一个EWOULDBLOCK的error.
	 *
	 * A. 默认情况下,close立即返回,如果sendbuffer还有数据,close将会等所有数据被发送完之后返回;
	 *     由于没有等待对方TCP的ACK信息,我们只能保证数据已经发送到对方,但不知道对方是否已经接受数据;
	 *     此时,TCP连接终止按照正常4次握手,经过TIME_WAIT状态;
	 * B. 0!=l_lineoff && l_linger是整数
	 *     close在收到对方TCP的ACK后才会返回(l_linger消耗完之前). 但该ACK信息只能保证对方已经收到数据,并不保证应用程序已经读取数据;
	 * C. l_linger值太小
	 *    由于l_linger值太小,sendbuffer发送完之前,close返回,返回-1 && errno=EWOULDBLOCK;
	 *    此种情况终止TCP连接,与l_linger=0类似,TCp连接终止不是按照正常4次握手,故TCP连接不会进入TIME_WAIT状态, 那么client会向server发送一个RST信息;
	 * D. shutdown,等待应用程序读取数据
	 *    同B对比,调用shutdown后紧接着调用read,此时read会阻塞,直到收到对方的FIN,即read是在对方调用close后返回的.
	 *    对方收到FIN终止请求后,会进入CLOSE_WAIT,对方断开TCP连接,需要调用一次close,也会发送一个FIN ACK,说明对方应用程序已经读取数据和FIN请求.
	 *    read在收到对方的FIN ACK之后返回.
	 *   
	 */
	/*@detail SO_REUSEADDR和SO_REUSEPORT
	 *
	 */
	 struct linger ling;
	 if(enable)
	 {
	     ling.l_onoff = 1;
		 ling.l_linger = second;
	 }
	 else
	 {
	     ling.l_onoff = 0;
		 ling.l_linger = 0;
	 }
	 int rc=setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *)&ling,sizeof(struct linger));
	 if(rc<0)
	     return errno;
	 return MXX_SOCKET_SUCC;
}

/**
 * @brief keepalive属性
 * @param
 * @note
 *    仅适用于TCP;
 *    大多数内核是基于整个内核维护该时间参数的,而不是基于每个socket维护;
 *    常用于服务器,用于检测半开连接并关闭;
 **/
int mxx_sockopt_set_keepalive_enable(int sockfd, bool enable )
{
    /*@detail keepalive说明
	 *  仅适用于TCP;
     *  大多数内核是基于整个内核维护该时间参数的,而不是基于每个socket维护;
	 *  常用于服务器,用于检测半开连接并关闭;
	 *   三个选项SOL_TCP: 
	 *   TCP_KEEPIDLE
	 *   TCP_KEEPINTVL
	 *   TCP_KEEPCNT
	 */
	 int rc;
	 int keepAlive = enable ? 1 : 0;   // 开启keepalive属性. 缺省值: 0(关闭)
	 rc=setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive)); 
     if(rc<0)
	     return errno;
	 return MXX_SOCKET_SUCC;
}

int mxx_sockopt_set_keepalive_idle(int sockfd, int keepIdle)
{
	int rc;
    //int keepIdle = 60;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)     
	rc=setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    if(rc<0)
        return errno;
    return 	MXX_SOCKET_SUCC;
}

int mxx_sockopt_set_keepalive_keepInterval(int sockfd, int keepInterval)
{
    int rc;
	//int keepInterval = 5;   // 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
	rc=setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
	if(rc<0)
        return errno;
    return 	MXX_SOCKET_SUCC;
}

int mxx_sockopt_set_keepalive_keepCount(int sockfd, int keepCount)
{
    int rc;
	//int keepCount = 2;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
	rc=setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
	if(rc<0)
        return errno;
    return 	MXX_SOCKET_SUCC;
}

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
int mxx_sockopt_set_keepalive_info(int sockfd, bool enable, int keepIdle/*=-1*/, int keepInterval/*=-1*/, int keepCount/*=-1*/)
{
    if(!enable)
        return 	mxx_sockopt_set_keepalive_enable(sockfd, false);
	else
	{
	    mxx_sockopt_set_keepalive_enable(sockfd, true);
		if(keepIdle>0)     mxx_sockopt_set_keepalive_idle(sockfd, keepIdle);
		if(keepInterval>0) mxx_sockopt_set_keepalive_keepInterval(sockfd, keepInterval);
		if(keepCount>0)    mxx_sockopt_set_keepalive_keepCount(sockfd, keepCount);
	}
	return 	MXX_SOCKET_SUCC;
 }

/**
 * @brief 从socket接收tcp数据,直到收到制定长度数据或者超时;
 * @param
 *   [in]sockfd
 *   [in]buff/buffsize: 缓存指针和缓存大小
 *   [out]rcv_len: 实际接收数据长度
 * @retval
 *    0-成功; MXX_SOCKET_BUFF_ERROR-缓冲区错误; MXX_SOCKET_CLOSED-socket关闭; MXX_SOCKET_TIMEOUT-超时; 
 *    其他小于0-错误;
 **/
int mxx_socket_recv_waitall(int sockfd, char *buff, int buffsize, int *rcv_len, int timeout/*=0*/)
{
   int rc;
   int error_code=0;
   int recvd_len=0;//已接收数据长度
   char *ptr=buff;
   timespec t1,t2;
   assert(NULL!=buff);
   assert(buffsize>0);
   
   *rcv_len=0;//设置返回数据长度
   
   if( (NULL==buff) || (buffsize<=0) )
      return MXX_SOCKET_BUFF_ERROR;

   //if(recv_timeout>0)
   //{
   //   mxx_socket_set_timeout(sockfd, recv_timeout, -1);
   //}

   os_lapsetime(&t1);//!< 开始接收时间  按照系统运行时间计算,不受系统时间影响
   while(recvd_len<buffsize)
   {
       /*
	    * @detils     recv说明 @see man recv
		* 1. 如果socket没有数据到达 && 没有设置nonblock, 则recv会等待数据到达;
		*               没有数据到达 && 设置nonblock, 则recv会立即返回, 并将errno设置为EAGAIN or EWOULDBLOCK;
		*    如果数据到达, recv调用通常返回的数据尽可能满足指定长度,而不是等待一定满足指定长度数据后才返回;
		*    可以使用select/epoll侦测socket是否有数据到达;
		* 2.MSG_WAITALL (since Linux 2.2)
		*    指明数据必须满足指定长度后才能返回; 
		*    如果signal中断||发生错误 || 连接出错 || 下一个接收数据与已接收数据类型不一致,则返回数据长度可能小于指定长度;
		* 3.MSG_DONTWAIT (since Linux 2.2)
        *      启用nonblock; 如果操作引起阻塞,则立即返回,并设置错误标记EAGAIN or EWOULDBLOCK;
		*	  另: 该标记也可以通过fcntl(F_SETFL,O_NONBLOCK))设置;
		* 4.MSG_OOB
        *      数据流不接收out-of-band data.  
		*	  部分协议会在数据队列头部设置expedited data, and thus this flag cannot be used with such protocols.
        * 5.MSG_PEEK
		*     获取接收队列中的数据,但是不会将数据从接收队里移除; 后续的读操作仍会获取这些数据;
        * 6.MSG_TRUNC(since Linux 2.2)
		*   tcp不支持该参数;
		*   对于raw(AF_PACKET),Internet datagram(linux2.4.27/2.6.8), netlink(since 2.6.22),返回封包实际长度,可能比传入的缓冲区大;
		* 7. 返回值:
		*    >0表示读取字节数; <0-错误; 0-peer关闭;
        * 默认socket的recv是阻塞的; 无论阻塞与非阻塞,rc<0表示失败; =0连接关闭; >0-收到数据;
        *     当数据没有到达,默认情况下(阻塞模式)recv会等待数据到达;
        * 注意: rc<0 &&((errno==EINTR)||(errno==EWOULDBLOCK)||(errno==EAGAIN)) 认为是正常的,继续接收数据
        *   当非阻塞模式下,如果socket没有可用数据,则返回-1,并将errno设置为EAGAIN或EWOULDBLOCK
        **/
       rc=recv(sockfd, ptr, buffsize-recvd_len,MSG_WAITALL);
       if(0==rc)//连接关闭
          return MXX_SOCKET_CLOSED;
       else if(rc<0)
       {
	       error_code=errno;//!< 防止多线程,读取后置0;
           //非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
           if( (EAGAIN==error_code) || (EWOULDBLOCK==error_code) ) //socket被标记为非阻塞,接收被阻塞或超时
		   {
             //break;
		   }
           else if(EINTR==error_code)//在收到数据前,被信号中断,则继续获取数据
           {
		   }
		   else      
               return error_code;
       }
	   else
	   {
           recvd_len +=rc;
           *rcv_len = recvd_len;
	   }
	   
	   //判断是否超时
	   if( (timeout>0) && (recvd_len<buffsize) )
	   {
	       os_lapsetime(&t2);
	       if(difftime_to_millisecond(&t1, &t2)>timeout)
		       return MXX_SOCKET_TIMEOUT;
	   }
   }//end of while

   return MXX_SOCKET_SUCC;
}

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
int mxx_socket_recv_nowait(int sockfd, char *buff, size_t buffsize, size_t *rcv_len)
{
   int rc;
   int error_code=0;
   int recvd_len=0;//已接收数据长度
   char *ptr=buff;

   assert(NULL!=buff);
   assert(buffsize>0);
   
   *rcv_len=0;//设置返回数据长度
   
   if( (NULL==buff) || (buffsize<=0) )
      return MXX_SOCKET_BUFF_ERROR;

   while((size_t)recvd_len<buffsize)
   {
       /*
	    * @detils     recv说明 @see man recv
		* @see mxx_socket_recv_waitall注释
        * 默认socket的recv是阻塞的; 无论阻塞与非阻塞,rc<0表示失败; =0连接关闭; >0-收到数据;
        *     当数据没有到达,默认情况下(阻塞模式)recv会等待数据到达;
        * 注意: rc<0 &&((errno==EINTR)||(errno==EWOULDBLOCK)||(errno==EAGAIN)) 认为是正常的,继续接收数据
        *   当非阻塞模式下,如果socket没有可用数据,则返回-1,并将errno设置为EAGAIN或EWOULDBLOCK
        **/
       rc=recv(sockfd, ptr, buffsize-recvd_len,MSG_DONTWAIT);
       if(0==rc)//连接关闭
          return MXX_SOCKET_CLOSED;
       else if(rc<0)
       {
	       error_code=errno;//!< 防止多线程,读取后置0;
           //非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
           if( (EAGAIN==error_code) || (EWOULDBLOCK==error_code) ) //socket被标记为非阻塞,接收被阻塞或超时
             break;
           else if(EINTR==error_code)//在收到数据前,被信号中断,则继续获取数据
           {
		   }
		   else      
               return error_code;
       }
	   else
	   {
           recvd_len +=rc;
           *rcv_len = recvd_len;
	   }
   }

   return MXX_SOCKET_SUCC;
}


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
int mxx_socket_send_waitall(int sockfd, const char* buff, int bufflen, int *snd_len,int timeout) 
{
    ssize_t rc;
    int unsnd_len = bufflen;
    const char *ptr = buff;
    int err_code=0;
    
    int retry_count=0;//重试次数
    timespec t1,t2;
    os_lapsetime(&t1);//!< 开始接收时间  按照系统运行时间计算,不受系统时间影响
    if(NULL!=snd_len) *snd_len=0;
    
    while(unsnd_len>0) 
    {
        /*@details send
         * 1. 如果消息太长,则返回错误EMSGSIZE,且不会传送任何消息;
         *    如果消息长度>缓冲区可用手数, 则send会阻塞;如果设置NONBLOCK,则立即返回并设置EAGAIN或EWOULDBLOCK;
         *    缓冲区充足,则数据拷贝到协议发送缓冲区,然后send返回实际拷贝字节数; copy错误,则返回SOCKET_ERROR;
         *    send等待过程中发送错误,则返回SOCKET_ERROR; 
         *    unix系统中,send在等待协议传送时连接断开,send进程还会收到SIGPIPE信号;
         *    拷贝成功,但在等待协议发送后发生错误,则下一个socket函数会返回SOCKET_ERROR;
         *    每一个除send外的Socket函数在执行的最开始总要先等待套接字的发送缓冲中的数据被协议传送完毕才能继续，如果在等待时出现网络错误，那么该Socket函数就返回 SOCKET_ERROR
         * 2.MSG_DONTWAIT (since Linux 2.2)
         *    启用非阻塞操作;如果可能阻塞,则返回EAGAIN或EWOULDBLOCK;
         *@note     Linux may return EPIPE instead of ENOTCONN
         */
        rc = send(sockfd, ptr, unsnd_len, 0);//如何确定错误码是上个send失败引起的???
        if(rc < 0) 
        {
            err_code=errno;
            //发送被信号中断,当send收到信号时,可以继续写;
            if(err_code == EINTR)
            {
                //continue;
            }
            // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
            // 在这里做延时后再重试.
            else if( (err_code == EAGAIN) ||(err_code == EWOULDBLOCK))
            {
                ++retry_count;
                if(retry_count>3)//重试超过次数,则退出
                    return -1;
                //usleep(1000);
                //continue;
            }
            else 
                return err_code;
            
            if((timeout>0)&&(unsnd_len>0))
            {
                //rc<0,不用考虑unsnd_len;
                os_lapsetime(&t2);
                if(difftime_to_millisecond(&t1, &t2)>timeout)
                    return MXX_SOCKET_TIMEOUT;
            }
        }

        if(NULL!=snd_len) *snd_len +=rc;
        
        unsnd_len -= rc;
        ptr       += rc;
    }
    
    return (bufflen-unsnd_len);
}

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
int mxx_socket_send_nowait(int sockfd, const char* buff, int bufflen, int *snd_len) 
{
    ssize_t rc;
    int unsnd_len = bufflen;
    const char *ptr = buff;
    int err_code=0;
    //int retry_count=0;//重试次数
    if(NULL!=snd_len) *snd_len=0;
    //int total_send_count=0;
    while(unsnd_len>0) 
    {
        /*@details send
         * 1. 如果消息太长,则返回错误EMSGSIZE,且不会传送任何消息;
         *    如果消息长度>缓冲区可用手数, 则send会阻塞;如果设置NONBLOCK,则立即返回并设置EAGAIN或EWOULDBLOCK;
         *    缓冲区充足,则数据拷贝到协议发送缓冲区,然后send返回实际拷贝字节数; copy错误,则返回SOCKET_ERROR;
         *    send等待过程中发送错误,则返回SOCKET_ERROR; 
         *    unix系统中,send在等待协议传送时连接断开,send进程还会收到SIGPIPE信号;
         *    拷贝成功,但在等待协议发送后发生错误,则下一个socket函数会返回SOCKET_ERROR;
         *    每一个除send外的Socket函数在执行的最开始总要先等待套接字的发送缓冲中的数据被协议传送完毕才能继续，如果在等待时出现网络错误，那么该Socket函数就返回 SOCKET_ERROR
         * 2.MSG_DONTWAIT (since Linux 2.2)
         *    启用非阻塞操作;如果可能阻塞,则返回EAGAIN或EWOULDBLOCK;
         *@note     Linux may return EPIPE instead of ENOTCONN
         */
        rc = send(sockfd, ptr, unsnd_len, MSG_DONTWAIT);
        if(rc < 0) 
        {
            err_code=errno;
            //发送被信号中断,当send收到信号时,可以继续写;
            if(err_code == EINTR)
            {
                continue;
            }
            // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
            // 在这里做延时后再重试.
            else if( (err_code == EAGAIN) ||(err_code == EWOULDBLOCK))//可能阻塞,就不要等了
            {
                //++retry_count;
                //if(retry_count>3)//重试超过次数,则退出
                //    return -1;
                //usleep(1000);
                //continue;
                break;
            }
            else 
                return err_code;
        }
        
        if(NULL!=snd_len) *snd_len +=rc;
        unsnd_len -= rc;
        ptr       += rc;
    }
    
    return (bufflen-unsnd_len);
}
