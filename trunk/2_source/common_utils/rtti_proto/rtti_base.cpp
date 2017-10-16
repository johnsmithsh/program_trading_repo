
#include <string.h>

#include "rtti_base.h"

 
//获取字段值
//参数:
//  [in]name:字段名
//  [out]data_ptr: 返回数据缓存区
//  [in]data_size: 返回缓存大小
//返回值:
//  0-成功; <0-失败;
int CRttiStBase::get_field_value(char *name, unsigned char *data_ptr, int size)
{
   if(NULL==data_ptr) return -1;
   
    memset(data_ptr, 0, size);//清空输出

   //获取当前数据包的字段描述器
   CFieldDescriptor *field_desc_ptr=get_field_desc();
   if(NULL==field_desc_ptr) return -2;
   
   //根据字段名获取字段信息
   ST_FIELD_DESC *field_info=field_desc_ptr->get_field_ptr(name);//获取字段描述
   unsigned char *addr = get_addr();//获取数据包起始指针
   if((NULL==field_info) || (NULL==addr)) return -5;

   memcpy(data_ptr, addr+field_info->offset, size>field_info->size ? field_info->size : size);
   return 0;
}

//设置字段值
//参数:
//  [in]name:字段名
//  [in]data_ptr: 返回数据缓存区
//  [in]data_size: 返回缓存大小
//返回值:
//  0-成功; <0-失败;
int CRttiBase::set_field_value(char *name, unsigned char *data_ptr, int size)
{
   if(NULL==data_ptr) return -1;

   //获取当前数据包的字段描述器
   CFieldDescriptor *field_desc_ptr=get_field_desc();
   if(NULL==field_desc_ptr) return -2;

   //根据字段名获取字段信息
   ST_FIELD_DESC *field_info=field_desc_ptr->get_field_ptr(name);//获取字段描述
   unsigned char *addr = get_addr();//获取数据包起始指针    
   if((NULL==field_info) || (NULL==addr)) return -5;

   memset(addr+field_info->offset, 0, field_info->size);
   memcpy(addr+field_info->offset, data_ptr, size>field_info->size ? field_info->size : size);

   return 0;
}



//序列化
int CRttiBase::serialize(unsigned char *buff,   int size)
{
   if( (NULL==buff) || (size<=0) ) 
      return -1;
   memset(buff, 0, size);

   CFieldDescriptor *field_desc_ptr=get_field_desc();//获取字段描述
   unsigned char *addr = get_addr();//获取数据包起始指针

   unsigned char *ptr=buff;
   CFieldDescriptor::iterator iter=field_desc_ptr->begin();
   CFieldDescriptor::iterator end_iter=field_desc_ptr->end();
   for(iter=field_desc_ptr->begin(); iter!=end_iter; iter++)//while(iter!=end_iter)
   {
      if((ptr+iter->size)>=(buff+size))//输出缓存不足
         return 0;
      memcpy(ptr, addr+iter->st_offset, iter->size);
      
      ptr += iter->size;
   }

   return (ptr-buff);//返回序列字节大小
}
//反序列化
int deserialize(unsigned char *buff, int size)
{
   if( (NULL==buff) || (size<=0) ) 
      return -1;
   //memset(buff, 0, size);

   CFieldDescriptor *field_desc_ptr=get_field_desc();//获取字段描述
   unsigned char *addr = get_addr();//获取数据包起始指针
   
   int st_size = field_desc_ptr->get_st_size();

   CFieldDescriptor::iterator iter=field_desc_ptr->begin();
   CFieldDescriptor::iterator end_iter=field_desc_ptr->end();
   int len;
   while(iter!=end_iter)
   {
      len = iter->size;
      if((iter->offset+iter->size)>=size)//buff数据不足
         len = size - iter->offset;
      memcpy(addr+iter->st_offset, buff+iter->offset, len);
   }

   return 0;
}
