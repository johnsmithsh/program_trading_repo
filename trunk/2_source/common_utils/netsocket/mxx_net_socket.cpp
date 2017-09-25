#include "mxx_net_socket.h"
#include <sys/epoll.h>

//#include <string.h>
//#include <stdlib.h>


#include <netinet/in.h>
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

//功能:删除创建的socket描述符
int mxx_socket_delete(int sockfd)
{
   return close(sockfd);
}

//功能: socket文件描述符作为server启动监听; 注:调用前必须保证sockfd完成,socker()->bind() 和setsockopt
//返回值:
//   0-成功;<0-失败;
int mxx_socket_listen(int sockfd, int max_conn)
{
   return listen(sockfd,max_conn);
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
int mxx_socket_listen(char *ip, int port,int max_conn, int recv_timeout/*=10*/, int send_timeout/*=10*/)
{
   int so_fd;
   int rc;

   //输入参数校验
   if(max_conn<=0)//最大连接数必须大于0
      return -1;
   if(port<=0)//端口号必须大于0
      return -1;
   
   //创建socket
   so_fd=socket(AF_INET, SOCK_STREAM, 0);
   if(so_fd<=0)
      return -1;

   //bind绑定
   struct sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family=AF_INET;
   /*
    * INADDR_ANY:任意地址; 
    * INADDR_NONE:inet_addr("255.255.255"),广播地址;
    * INADDR_LOOPBACK:inet_addr("127.0.0.1")
    * */
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

   //设置超时时间
   rc=mxx_socket_set_timeout(so_fd, recv_timeout, send_timeout);
   if(rc<0)
   {
     close(so_fd);
      return -3;
   }

   //启动监听
   rc=listen(so_fd,max_conn);
   if(rc<0)//启动监听失败
   {
     close(so_fd);
      return -4;
   }

   
   return so_fd;
}


//注:
//   accept前连接断开的情况,尚未想好怎么处理;
int mxx_socket_accept();

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
int mxx_socket_set_timeout(int so, int recv_timeout, int send_timeout)
{
   struct timeval timeout={0,0};
   int rc;

   if(recv_timeout>0)
   {
      timeout.tv_sec=__mxx_intpart_(recv_timeout/1000);
      timeout.tv_usec=(recv_timeout%1000)*1000;//微妙
      rc=setsockopt(so, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
      if(rc<0)
         return -1;
   }

   if(send_timeout>0)
   {
      timeout.tv_sec=__mxx_intpart_(send_timeout/1000);
      timeout.tv_usec=(send_timeout%1000)*1000;//微妙
      rc=setsockopt(so, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
      if(rc<0)
         return -1;
   }

  return 0;
   
}

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

   if( (NULL==buffer) || (buffsize<=0) )
      return -1;

   if(recv_timeout>0)
   {
      mxx_socket_set_timeout(sockfd, recv_timeout, -1);
   }

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
           //非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
           if( (errno==EAGAIN) || (EWOULDBLOCK==errno) ) //socket被标记为非阻塞,接收被阻塞或超时
             break;
           else if(EINTR==errno)//在收到数据前,被信号中断
             continue;
           return -1;
       }
       recvd_len +=rc;
       *data_len = recvd_len;
   }

   return 0;
}

//使用socket发送数据 tcp
int mxx_socket_send(int sockfd, const char* buffer, int buflen) 
{
  ssize_t rc;
  size_t total = buflen;
  const char *p = buffer;
  int retry_count=0;//重试次数
  while(1) {
    rc = send(sockfd, p, total, 0);
    if(rc < 0) {
      // 当send收到信号时,可以继续写,但这里返回-1.
      if(errno == EINTR)
        return -1;
      // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
      // 在这里做延时后再重试.
      if(errno == EAGAIN) {
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


int mxx_socket_set_nonblock(int sockfd)
{
   int flags;
   flags = fcntl(sockfd, F_GETFL, 0);
   if(-1==flags)
      return -1;
   flags=fcntl(sockfd, flags|O_NONBLOCK);
   if(-1==flags)
     return -2;
   return 0;
}

int mxx_socket_get_nonblock(int sockfd)
{
   int flags;
   flags = fcntl(sockfd, F_GETFL, 0);
   if(-1==flags)
      return -1;
   return (flags & O_NONBLOCK) ? 1 : 0;
}
