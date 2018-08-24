#ifndef __RTTI_ST_BASE_H_
#defien __RTTI_ST_BASE_H_

#include "rtti_def.h"

//定义数据包基类: 所有的运行时struct都必须从此处继承
//子类必须实现:
//   get_addr();//获取struct数据包地址指针
//   get_size(); //获取struct数据大小
class CRttiStBase
{
   public:
      CRttiStBase() { m_ptr_descriptor = NULL; }
      CRttiStBase(CFieldDescriptor *field_desc_ptr) : m_ptr_descriptor(field_desc_ptr){}

      //不必实现拷贝构造函数
      //不必实现赋值函数

   public:    
      //返回获取数据包指针,即数据结构第一个字段的指针,
      //纯虚函数: 子类实现
      virutal unsigned char *get_addr() = 0;
      //返回数据结构大小(struct结构大小)
      //纯虚函数: 子类实现
      virtual int get_size() = 0;
 
      //获取字段值
      int get_field_value(char *name, unsigned char *data_ptr, int size);
      //设置字段值
      int set_field_value(char *name, unsigned char *data_ptr, int size);

      //@brief 获取字段值
      char           get_char(char *name)  {          char  value=0; get_field_value(name, &value, sizeof(value)); return value; }
      unsigned char  get_uchar(char *name) { unsigned char  value=0; get_field_value(name, &value, sizeof(value)); return value; }
      short          get_int2(char *name)  {          short value=0; get_field_value(name, &value, sizeof(value)); return value; }
      unsigned short get_uint2(char *name) { unsigned short value=0; get_field_value(name, &value, sizeof(value)); return value; }
      int            get_int4(char *name)  {          int   value=0; get_field_value(name, &value, sizeof(value)); return value; }
      unsigned int   get_uint4(char *name) { unsigned int   value=0; get_field_value(name, &value, sizeof(value)); return value; }
      long           get_long(char *name)  { long           value=0; get_field_value(name, &value, sizeof(value)); return value; }
      double         get_double(char *name){ double       value=0.0; get_field_value(name, &value, sizeof(value)); return value; }
      char*          get_string(char *name, unsigned char *buff_ptr, int size) 
      { 
         if(NULL!=buff_ptr) *buff_ptr='\0';
         get_field_value(name, buff_ptr, size); 
         return buff_ptr; 
      }

      //@brief 根据字段名设置字段值
      int set_char(char   *name, char           value) { return set_field_value(name, &value, sizeof(value)); }
      int set_uchar(char  *name, unsigned char  value) { return set_field_value(name, &value, sizeof(value)); }
      int set_int2(char   *name, short          value) { return set_field_value(name, &value, sizeof(value)); }
      int set_uint2(char  *name, unsigned short value) { return set_field_value(name, &value, sizeof(value)); }
      int set_int4(char   *name,  int           value) { return set_field_value(name, &value, sizeof(value)); }
      int set_uint4(char  *name, unsigned int   value) { return set_field_value(name, &value, sizeof(value)); }
      int set_long(char   *name, long           value) { return set_field_value(name, &value, sizeof(value)); }
      int set_double(char *name, double         value) { return set_field_value(name, &value, sizeof(double)); }
      int set_string(char *name, unsigned char *value, int len) { return set_field_value(name, &value, len); }

   public:
      //序列化
      int serialize(unsigned char *buff,   int size);
      //反序列化
      int deserialize(unsigned char *buff, int size);
   public:
      //功能: 返回数据包字段描述信息
      CFieldDescriptor *get_field_desc() { return m_ptr_descriptor; }
   protected:
      CFieldDescriptor *m_ptr_descriptor;//字段描述器; 描述当前数据结构下所有字段信息;
};

#endif
