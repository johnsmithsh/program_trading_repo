
#include <string.h>

#include "rtti_def.h"

//////////////////////////////////////////迭代器实现///////////////////////////////////////////////////////////

//////////////////////////////////////////迭代器实现完成////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//实现数据包描述器: 描述一个数据包中的字段信息,不支持嵌套; 如果嵌套还是使用proto buffer吧!
//
//
CFieldDescriptor::CFieldDescriptor()
{
   m_field_list.clear();
}

CFieldDescriptor::~CFieldDescriptor()
{
   m_field_list.clear();
}

int CFieldDescriptor::append_field(ST_FIELD_DESC::RTTI_DATA_TYPE data_type, char *name, char *comment, int st_offset, int offset, int size, int count)
{
  ST_FIELD_DESC field_desc;
  //校验数据类型是否合法
  if(     (ST_FIELD_DESC::RDT_CHAR!=data_type) 
       && (ST_FIELD_DESC::RDT_CHAR!=data_type)
       && (ST_FIELD_DESC::RDT_UCHAR!=data_type)
       && (ST_FIELD_DESC::RDT_INT2!=data_type)
       && (ST_FIELD_DESC::RDT_UINT2!=data_type)
       && (ST_FIELD_DESC::RDT_INT4!=data_type)
       && (ST_FIELD_DESC::RDT_UINT4!=data_type)
       && (ST_FIELD_DESC::RDT_LONG!=data_type)
       && (ST_FIELD_DESC::RDT_DOUBLE!=data_type)
       && (ST_FIELD_DESC::RDT_STR!=data_type)
       && (ST_FIELD_DESC::RDT_VAR_STR!=data_type)
    )
    return -1;
 //校验名字是否合法
 if((NULL==name) || ('\0'==*name)) return -2;
 for(int i =0; i<m_field_list.size(); i++)
 {
    if(0==strcmp(name, m_field_list[i].field_name))//字段名已经存在
        return -3;
 }

 memset(&field_desc, 0, sizeof(field_desc));
 field_desc.field_index = m_field_list.size();
 field_desc.field_data_type = data_type;
 strncpy(field_desc.field_name, name, sizeof(field_desc.field_name)-1);
 if(NULL!=comment) strncpy(field_desc.field_comment, comment, sizeof(field_desc.field_comment)-1);
 field_desc.count=count;
 field_desc.st_offset = st_offset;
 field_desc.proto_offset = offset;
 field_desc.field_size = size;

 m_field_list.push_back(field_desc);
 return 0; 
}

int CFieldDescriptor::append_field(ST_FIELD_DESC::RTTI_DATA_TYPE data_type, char *name, char *comment, int st_offset, int size)
{
   //根据上一个字段的偏移+大小,计算当前字段的偏移
   //注: 此处的offset用于序列化
   int offset=0;
   int index = m_field_list.size();
   if(index>0)
   {
      ST_FIELD_DESC *field_ptr= &m_field_list[index-1];
      offset = field_ptr->st_offset + field_ptr->field_size;
   }
   
   return this->append_field(data_type, name, comment, st_offset, offset, size, 1);
}

ST_FIELD_DESC * CFieldDescriptor::add_field(char *name, char *comment, int st_offset, int size)
{
  ST_FIELD_DESC field_desc;
  memset(&field_desc, 0, sizeof(field_desc));
  field_desc.field_index      = m_field_list.size();
  field_desc.field_data_type  = st_field_t::RDT_UNKONOWN;
  strncpy(field_desc.field_name,       name,     sizeof(field_desc.field_name)-1);
  if(NULL!=comment) 
      strncpy(field_desc.field_comment, comment, sizeof(field_desc.field_comment)-1);
  field_desc.count      = 1;
  field_desc.st_offset  = st_offset;
  field_desc.proto_offset     = st_offset;
  field_desc.field_size       = size;
  m_field_list.push_back(field_desc);
  
  return &(*(m_field_list.rbegin()));
}

/**
 * 功能: 根据索引查找字段定义
 * 参数:
 *   [in]index:字段索引
 *   [out]field_desc:返回找到的字段
 * 返回值:
 *    0-成功; <0-失败; 
 **/
int CFieldDescriptor::find_field(int index, ST_FIELD_DESC *field_desc)
{
  ST_FIELD_DESC * ptr = find_field_ptr(index);
  if(NULL==ptr) return -1;
  if(NULL!=field_desc) *field_desc = *ptr;
  return 0;
}
ST_FIELD_DESC & CFieldDescriptor::find_field(int index)
{
  ST_FIELD_DESC * ptr = find_field_ptr(index);
  //if(NULL==ptr) return 0;
  return *ptr;
}
ST_FIELD_DESC *CFieldDescriptor::find_field_ptr(int index)
{
  if( (index<0) || index>=m_field_list.size()) return NULL;
  return &m_field_list[index];
}

/**
 * 功能: 根据索引查找字段定义
 * 参数:
 *   [in]name:字段名
 *   [out]field_desc:返回找到的字段
 * 返回值:
 *    0-成功; <0-失败;
 **/
int CFieldDescriptor::find_field(char *name, ST_FIELD_DESC *field_desc)
{
  ST_FIELD_DESC * ptr = find_field_ptr(name);
  if(NULL==ptr) return -1;
  if(NULL!=field_desc) *field_desc = *ptr;
  return 0;
}

ST_FIELD_DESC * CFieldDescriptor::find_field_ptr(char *name)
{
  int count=m_field_list.size();
  for(int i=0; i< count; i++)
  {
     if(0==strcmp(name, m_field_list[i].field_name))
       return &m_field_list[i];
  }
  return NULL;

}


//迭代器相关
CFieldDescriptor::iterator CFieldDescriptor::begin()
{
   CFieldDescriptor::iterator iter(this);
   iter.set_cursor(0);
   return iter;
}

CFieldDescriptor::iterator CFieldDescriptor::end()
{
  CFieldDescriptor::iterator iter(this);
   iter.set_cursor(-1);
   return iter;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
