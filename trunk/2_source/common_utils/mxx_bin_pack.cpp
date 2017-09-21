#include "mxx_bin_pack.h"

#include <string.h>
#include <stdlib.h>

//功能:分配一个二进制协议缓存
ST_BIN_BUFF *mxx_alloc_bin_pack(int data_len)
{
    unsigned char *bin_ptr=NULL;
    if(data_len<=BIN_PACK_MIN_SIZE)
       data_len=BIN_PACK_MIN_SIZE;
    if(data_len>=BIN_PACK_MAX_SIZE)
       data_len=BIN_PACK_MAX_SIZE;
    int pack_size=data_len+sizeof(ST_BIN_BUFF);//需要申请空间大小

    bin_ptr=(unsigned char *)malloc(pack_size);
    if(NULL==bin_ptr)  return NULL;

    memset(bin_ptr, 0, pack_size);
    
    ST_BIN_BUFF * bin_buff_ptr=(ST_BIN_BUFF *)bin_ptr;
    bin_buff_ptr->bin_pack_buff_size=data_len+sizeof(ST_BIN_DATA_HEAD);//设置缓存大小
    bin_buff_ptr->bin_pack_ptr=(BIN_PACK_Ptr)&(bin_buff_ptr->bin_pack_data_len);//从bin_buff_ptr->data_len之后就是bin_pack数据包
    bin_buff_ptr->bin_pack_data_len.bin_data_len=0;//数据长度
    bin_buff_ptr->cur_data_len=0;//当前缓存中实际数据长度
}

//功能: 释放二进制协议缓存
void mxx_free_bin_pack(ST_BIN_BUFF * bin_buff)
{
   if(NULL!=bin_pack)
     free(bin_pack);
}

//功能: 清空BIN_PACK;
void mxx_bin_pack_clear(ST_BIN_BUFF *bin_buff)
{
   if(NULL==bin_buff) return;

   int data_len=bin_buff_ptr->bin_pack_data_len.bin_data_len;
   //bin_buff_ptr->bin_pack_buff_size=data_len+sizeof(ST_BIN_DATA_HEAD);//设置缓存大小
   //bin_buff_ptr->bin_pack_ptr=(BIN_PACK_Ptr)&(bin_buff_ptr->bin_pack_data_len);//从bin_buff_ptr->data_len之后就是bin_pack数据包
   //bin_buff_ptr->bin_pack_data_len.bin_data_len=0;//数据长度
   bin_buff_ptr->cur_data_len=0;//当前缓存中实际数据长度
   memset(bin_buff_ptr->bin_pack_ptr, 0, data_len);
   return ;
}

//功能: 向缓存添加数据
int mxx_bin_pack_append(ST_BIN_BUFF *bin_buff, unsigned char *data_buff, int data_len)
{
   if( (NULL=bin_buff) || (NULL==data_buff) || (data_len<=0) )
       return -1;
   int avail_size=mxx_bin_pack_get_avail_size(bin_buff);
   if(avail_size<=0)
      return -2;
   if(avail_size<data_len)//缓存可用空间不足;
      data_len = avail_size;

   memcpy(bin_buff_ptr->bin_pack_ptr, data_buff, data_len);
   bin_buff_ptr->bin_pack_data_len.bin_data_len += data_len;//更改长度
   return data_len;
}

//功能:  获取缓冲区可用长度
int mxx_bin_pack_get_avail_size(ST_BIN_BUFF *bin_buff)
{
   if(NULL==bin_buff) return 0;
   return bin_buff->bin_pack_buff_size-sizeof(ST_BIN_DATA_HEAD)-bin_buff->bin_pack_data_len.bin_data_len;
}

//功能: 判断缓存区是否已经满了;
bool mxx_bin_pack_is_full(ST_BIN_BUFF *bin_buff)
{
   return mxx_bin_pack_get_avail_size(bin_buff)<=0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//从缓存中获取BIN_PACK的数据指针
BIN_PACK_Ptr mxx_bin_pack_get_ptr(ST_BIN_BUFF *bin_buff)
{
    return (NULL==bin_buff) ? NULL : bin_buff->bin_pack_ptr;
}

//获取BIN_PACK协议包大小,包括报文头
int mxx_bin_pack_getsize(ST_BIN_BUFF *bin_buff)
{
    return (NULL==bin_buff) ? 0: bin_buff->bin_pack_data_len.bin_data_len+sizeof(ST_BIN_DATA_HEAD);
}
