
#include "mxx_socket_event.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>

//收到读数据事件处理函数;
//一次必须读完一个完整包
//int mxx_read_event(int so, ST_SOCKET_BUFF_INFO *sock_buff_info, ST_BIN_BUFF **recv_buff )
int mxx_read_event(int so, ST_SOCKET_BUFF_INFO *sock_buff_info)
{
    
    ST_BIN_DATA_HEAD bin_head;
    bin_head.bin_data_len=0;
    int n;
    //errno是线程安全的;
    //int sys_errno;//错误码,及时读取errno,防止多线程被篡改;
    
    int data_len = 0;//报文长度;
    ST_BIN_BUFF *bin_buff=NULL;

    if(NULL==sock_buff_info->buff)//需要读取一个新报文
    {
      //读取报文头,其中包含数据长度
      //此处使用阻塞模式读取,如果连报文头都读不出来,那就阻塞这里吧;
      n=recv(so, &bin_head, sizeof(bin_head),0);
      if(0==n)//对方关闭网络连接了
         return MXX_ERR_CONN_CLOSE;
      else if(n<=0)
      {
      }

      if(n<(int)sizeof(bin_head))//读取长度小于报文头长度
         return MXX_ERR_BIN_PACK_HEAD_UNCOMPLETE;
    
      if(bin_head.bin_data_len<BIN_PACK_MIN_SIZE)//报文过短
         return MXX_ERR_BIN_PACK_HEAD_TOSMALL;
      else if(bin_head.bin_data_len>BIN_PACK_MAX_SIZE)//报文过长
         return MXX_ERR_BIN_PACK_HEAD_TOLONG;

      data_len = bin_head.bin_data_len;
      bin_buff=mxx_alloc_bin_pack(data_len);
      if(NULL==bin_buff)//缓存申请失败
         return MXX_ERR_BIN_PACK_ALLOC_FAILED;

      sock_buff_info->buff=NULL;
      sock_buff_info->buff_size=0;
      sock_buff_info->data_len=0;
      sock_buff_info->total_proc_len = 0;
    }
    else //上次报文没有读取完整,接着读
    {
       data_len = sock_buff_info->data_len - sock_buff_info->total_proc_len;
       bin_buff=sock_buff_info->buff;
    }

    //每次都要申请
    //int data_len = bin_head.bin_data_len;
    //ST_BIN_BUFF *bin_buff=NULL;
    //bin_buff=mxx_alloc_bin_pack(data_len);
    //if(NULL==bin_buff)//缓存申请失败
    //  return MXX_ERR_BIN_PACK_ALLOC_FAILED;
    
    //开始读取
    int timeout=1000;//超时时间,单位:毫秒
    int reading_time=0;//当前已经读取的时间,单位:秒
    int ret_code=0;//返回错误码
    bool succ_flag=false;//读取完成标记; true报文读取成功; false-报文读取失败
    int sum_count=0;//读取数据计数
    unsigned char *buff_ptr=mxx_bin_pack_data_ptr(bin_buff);//只要缓存申请成功,则一定能找到协议数据缓存指针
    while(data_len>0)
    {
       n=recv(so, buff_ptr, data_len, 0);
       if(n<0)
       {
          if((EAGAIN==errno) || (EWOULDBLOCK==errno))//没有可用数据
          {
             if(reading_time>timeout)//超时了
             {
                ret_code=MXX_ERR_BIN_PACK_UNCOMPLETE;
                break;
             }
             continue;
          }
          else if(EINTR==errno)//被中断,继续读取
          {
             continue;
          }
          else{//其他错误;
             ret_code=MXX_ERROR_SOCKET_RECV_FAILED;
             break;
          }
       }
       else if(0==n)//对方连接关闭
       {
          ret_code=MXX_ERR_CONN_CLOSE;
          break;
       }
       buff_ptr += n;
       data_len -= n;
       sum_count +=n;
   }//end of while(data_len>0)

   //sock_buff_info->data_len - sock_buff_info->total_proc_len;
   if(NULL==sock_buff_info->buff)//说明是读取的是新协议包
   {
      if(data_len>0)//没有读取完整,则释放缓存 
         mxx_free_bin_pack(bin_buff);
      else//协议包读取完整
      {
        sock_buff_info->buff = bin_buff;
        sock_buff_info->buff_size=mxx_bin_pack_size(bin_buff);
        sock_buff_info->data_len=sum_count;
        sock_buff_info->total_proc_len=sock_buff_info->data_len;
      }
   }
   else //读取是上次未读完的协议包
   {
         //sock_buff_info->buff = bin_buff;
         //sock_buff_info->buff_size=;
         //sock_buff_info->data_len=sum_count;
         sock_buff_info->total_proc_len += sum_count;
   }

   return ret_code;
}

//收到写数据事件处理函数
//int mxx_write_event(int so,ST_SOCKET_BUFF_INFO *snd_buff_info, ST_BIN_BUFF *bin_buff)
int mxx_write_event(int so,ST_SOCKET_BUFF_INFO *snd_buff_info)
{
   ST_BIN_BUFF *bin_buff=snd_buff_info->buff;
   if(NULL==bin_buff) return MXX_SUCC_CODE;
   //数据缓存不存在
   if( (NULL==bin_buff->bin_pack_ptr) || (bin_buff->bin_head.bin_data_len <= 0) ) return MXX_SUCC_CODE;

   unsigned char *pack_ptr=(unsigned char *)mxx_bin_pack_ptr(bin_buff);//获取报文头指针
   int pack_len=mxx_bin_pack_size(bin_buff);//获取pack大小

   if( (NULL==pack_ptr) || (pack_len<=0))
      return MXX_SUCC_CODE;

   int timeout=1000;//超时间,单位:毫秒;
   int lapse_time=0;//已经用过的时间
   int n;
   int sum_count=0;
   int ret_code=0;
   while(pack_len>0)
   {
      n=send(so, pack_ptr, pack_len,0);
      if(n<0)
      {
         if( (EAGAIN==errno) || (EWOULDBLOCK==errno) )//非阻塞模式出现
         {
             if(timeout<=0)
             {
                 ret_code=MXX_SEND_BUFF_FULL;
                 break;
             }
             else if(lapse_time>=timeout)
             {
                ret_code=MXX_ERR_TIMEOUT;
                break;
             }
         }
         else if(ECONNRESET==errno) //连接无效,比如对方服务或机器重启
         {
             ret_code=MXX_ERR_CONN_CLOSE;
             break;
         }
         else if(EINTR==errno)//数据传输前被signal中断
         {
            continue;
         }
         else if(ENOBUFS==errno)//发送缓存满了
         {
            continue;
         }
         else if(ENOTCONN==errno)//socket未连接
         {
            ret_code=MXX_ERR_CONN_CLOSE;
            break;
         }
         else if(EPIPE==errno)//本地终端关闭socket(cliented)连接
         {
            ret_code=MXX_ERR_CONN_CLOSE;
            break;
         }
         ret_code=MXX_SEND_ERROR;//发送错误
         break;
      }
      else if(0==n)
      {
      }
      else
      {
         pack_len -= n;
         pack_ptr += n;
         sum_count += n;
      }

   }
   
   if(pack_len>0)
   {
   }

   if(ret_code==0)
     return sum_count;
   else
     return ret_code;
}


