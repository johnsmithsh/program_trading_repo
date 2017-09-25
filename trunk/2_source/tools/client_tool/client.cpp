
/*
 * 测试工具:作为客户端工具向服务器发送请求;
 *
 *
 * */

#include "mxx_bin_pack.h"
#include "mxx_net_socket.h"

#define OUT_INFO_MSG(format,...) printf(format"\n", ##__VA_ARGS__)

char g_server_ip[16]={0};
int g_server_port=0;

//从标准输入获取一行;返回读取的字符数,<0-失败;
int read_line(char *buff, int size)
{
   memset(buff, 0, size);
   char *ptr=gets(buff);
   if(NULL==ptr)
      return -1;
   return strlen(buff);
}


int main(int argc, char ** argv)
{
   char line_buff[1024];
   int rc;
   int so;

   ST_BIN_BUFF *bin_pack=NULL;
   bin_pack=mxx_alloc_bin_pack(1024);
   if(NULL==bin_pack)
   {
      OUT_INFO_MSG("申请bin pack缓存失败!");
      exit(2);
   }

   so=mxx_socket_connect(g_server_ip, g_server_port);//连接到指定客户端
   if(so<0)
   {
      if(NULL1=bin_pack)
      {
         mxx_free_bin_pack(bin_pack);
         bin_pack=NULL;
      }
      OUT_INFO_MSG("连接ip:port=[%s:%d]!", g_server_ip, g_server_port);
      exit(3);
   }
   
   for(;;)
   {
      if((rc=read_line(line_buff, sizeof(line_buff)))<0)
      {
         OUT_INFO_MSG("Error: 读取输入失败\n");
         break;
      }
      else if(0==rc)
         continue;

      mxx_bin_pack_clear(bin_pack);
      mxx_bin_pack_append(bin_pack, line_buff, rc);

      unsignec char * ptr=mxx_bin_pack_ptr(bin_pack);
      int len=mxx_bin_pack_len(bin_pack);
      rc=mxx_socket_send(so, ptr, len);
      if(rc<0)
      {
         OUT_INFO_MSG("Error: 发送失败!rc=[%d]", rc);
         break;
      }
      else if(rc==len)
      {
         OUT_INFO_MSG("Error: 发送不完整,rc=[%d] != [%d]", rc, len);
      }
   }

   if(NULL==bin_buff)
   {
      mxx_free_bin_pack(bin_buff);
      bin_buff=NULL;
   }
   if(so > 0)
   {
      close(so);
   }

   return 0;
}
