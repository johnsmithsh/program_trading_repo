#ifndef MXX_RTTI_DATA_TYPE_H_
#define MXX_RTTI_DATA_TYPE_H_

#include "rtti_macro.h"
////////////////////////////////////////////////////////////

BEGIN_RTTI_NAMESPACE

//
//
//支持的数据类型
//enum RTTI_DATA_TYPE
//{
//   RDT_UNKONOWN=0,
//   RDT_CHAR=1,
//   RDT_UCHAR=2,
//   RDT_INT2=3,
//   RDT_UINT2=4,
//   RDT_INT4=5,
//   RDT_UINT4=6, //
//   RDT_LONG=7, //long
//   RDT_DOUBLE=8, //double
//   RDT_STR=9, //字符串
//   RDT_VAR_STR=10 //变长字符串
//};


//字段信息描述
typedef struct __st_field
{
    //支持的数据类型
    enum RTTI_DATA_TYPE
    {
       RDT_UNKONOWN=0,
       RDT_CHAR=1,
       RDT_UCHAR=2,
       RDT_INT2=3,
       RDT_UINT2=4,
       RDT_INT4=5,
       RDT_UINT4=6, //
       RDT_LONG=7, //long
       RDT_ULONG=8,
       RDT_FLOAT=9,
       RDT_DOUBLE=9, //double
       RDT_STR=10, //字符串 char []
       RDT_VAR_STR=11, //变长字符串
       RDT_struct =12,
       RDT_stdStr =13,//std::string
    };
    
    int            field_index;//字段索引,表示字段在struct中的定义顺序;
    RTTI_DATA_TYPE field_data_type;//字段类型
    char           field_name[64];//字段在pack中的名字
    char           field_comment[64];//字段说明,
    int            count;//元素个数,用于表示一个数组 
  
    int            st_offset;       //字段在struct中的偏移(struct定义) 
    int            proto_offset;   //字段在序列流中的偏移(用于结构体序列化)
    int            size;           //字段大小;

    struct __st_field *prev;
    struct __st_field *next;
   
}ST_FIELD_DESC,st_field_t;

END_RTTI_NAMESPACE

#endif
