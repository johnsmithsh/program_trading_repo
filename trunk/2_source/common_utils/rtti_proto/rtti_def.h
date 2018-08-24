#ifndef MXX_RTTI_DEFINE_H_
#define MXX_RTTI_DEFINE_H_

#include <vector>
#include "rtti_macro.h"
/*
 * 定义数据结构struct运行时说明信息
 * 用于接口协议定义
 *
 *  根据fastdb的class.h改写
 *  为了在程序中同时使用fastdb与rtti,宏定义需要区分,最好有单独的命名空间,与fastdb宏定义区分开
 *
 *  名字说明:
 *     data_type: 数据类型
 *     field: 字段,struct每个字段定义
 *     pack:  struct结构体,没有嵌套定义
 **/
////////////////////////////////////////////////////////////

/**
 * 宏用于描述一个字段信息
 * 定义一个变量并设置字段类型
 */
#define RTTI_FIELD(x, index) \
    (*RTTI_NS::setDescribeFieldType(new RTTI_NS::rttiFieldDescriptor(#x, (char*)&x-(char*)this, \
                                                                  sizeof(x), index), x))

/**
 * Macro used to describe fields of the struct. Use <code>RTTI_FIELD</code>...
 * macros separated by comma inside this macro to describe all fields of the struct
 * 宏用于描述结构体每个字段信息
 * 定义一个结构体变量,描述一个结构体下每个字段信息
 * 定义一个函数,该函数向结构体添加每个字段的信息;
 */
#define RTTI_TYPE_DESCRIPTOR(fields) \
    RTTI_NS::rttiFieldDescriptor* rttiDescribeComponents(RTTI_NS:rttiFieldDescriptor*) { \
        return &fields; \
    } \
    static RTTI_NS::rttiStDescriptor stDescriptor

    
#define RTTI_REGISTER_IN(st_name, database) \
    GET_TABLE_DESC_PREFIX RTTI_NS::dbTableDescriptor* dbGetTableDescriptor GET_TABLE_DESC_PARAM(table) \
      { return &table::dbDescriptor; }            \
    static RTTI_NS::dbFieldDescriptor* dbDescribeComponentsOf##table() \
      { return ((table*)0)->dbDescribeComponents(NULL); }     \
    RTTI_NS::dbTableDescriptor table::dbDescriptor(#table, database, sizeof(table), \
                                          &dbDescribeComponentsOf##table)
////////////////////////////////////////////////////////////
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
       RDT_struct =12, //结构体变量
       RDT_stdStr =13,//std::string
    };
    
    int            field_index;//字段索引,表示字段在struct中的定义顺序;
    RTTI_DATA_TYPE field_data_type;//字段类型
    char           field_name[64];//字段在pack中的名字
    char           field_comment[64];//字段说明,
    int            count;//元素个数,用于表示一个数组 
  
    int            st_offset;       //字段在struct中的偏移(struct定义) 
    int            proto_offset;   //字段在序列流中的偏移(用于结构体序列化)
    int            field_size;     //字段大小;

    struct __st_field *prev;
    struct __st_field *next;
   
}ST_FIELD_DESC,st_field_t;
//////////////////////////////////////////////////////////
//第一种实现方式
//定义一个字段信息类
class rttiFieldDescriptor
{
  public:
    rttiFieldDescriptor(const char *field_name, size_t offset, size_t size, int index);
    virtual ~rttiFieldDescriptor();
    
  public:
    /**
     * @brief 记录类实例在数据库记录中的大小
     * @param
     *    [in]base:数据表类实例的指针
     *    [in]offs:记录固定部分大小,如记录头???
     * @note
     *   dbFieldDescriptor::next与dbFieldDescriptor::prev构成一个环;
     *   fastdb实现比较巧妙,采用next知道next==this时停止,按照环的方式处理;
     *   这种方式不支持随机读写;
     *   由于struct不包括指针,故此处可以不用实现;
     **/
    //size_t calculateRecordSize(byte* base, size_t offs);
    //size_t calculateStructSize();
    
    st_field_t *get_field_info() { return &m_field; }
    
    //@brief 逗号运算符: 构建一个链表
    //@note field的prev和next必须有意义,如果是单个节点,则prev=next=this;
    rttiFieldDescriptor& operator, (rttiFieldDescriptor& field) 
    {
        rttiFieldDescriptor* tail = field.prev;
        tail->next = this;
        prev->next = &field;
        field.prev = prev;
        prev = tail;
        return *this;
    }
    //@brief 程序启动后不会修改字段,故不考虑移除情况
  public:
    //@brief 链表操作: 根据字段名找到对应的描述信息
    rttiFieldDescriptor *find_field(const char *name);
    //@brief 链表操作: 根据索引找到对应的描述信息
    rttiFieldDescriptor *find_field(int index);
    
    //@brief 获取序列化大小
    int get_serial_size();
    //获取struct结构体大小
    int get_st_size();
    
    //@brief 链表操作: serial数据转换为应用程序使用的struct数据
    void *convert_serial2struct(unsigned char *src, size_t src_size, unsigned char *dst, size_t dst_size);
    //@brief 链表操作: struct数据转换为serial数据
    void *convert_struct2serial(unsigned char *src, size_t src_size, unsigned char *dst, size_t dst_size);
    
  public:
    st_field_t m_field;
    
    rttiFieldDescriptor *prev;
    rttiFieldDescriptor *next;
    
    //rttiFieldDescriptor* components;//subbcomponents of the fields
};

//template<class T>
//inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, T& x)
//{
//    fd->field_data_type = st_field_t::RDT_struct;
//    if ((fd->components = x.dbDescribeComponents(fd)) != NULL) {
//        fd->elemSize = fd->components->dbsSize;
//    }
//    return fd;
//}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, char&)
{
    fd->field_data_type = st_field_t::RDT_CHAR;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, unsigned char&)
{
    fd->field_data_type = st_field_t::RDT_UCHAR;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, short&)
{
    fd->field_data_type = st_field_t::RDT_INT2;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, unsigned short&)
{
    fd->field_data_type = st_field_t::RDT_UINT2;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, int&)
{
    fd->field_data_type = st_field_t::RDT_INT4;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, unsigned int&)
{
    fd->field_data_type = st_field_t::RDT_UINT4;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, long&)
{
    fd->field_data_type = st_field_t::RDT_LONG;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, unsigned long&)
{
    fd->field_data_type = st_field_t::RDT_ULONG;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, float&)
{
    fd->field_data_type = st_field_t::RDT_FLOAT;
    return fd;
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, double&)
{
    fd->field_data_type = st_field_t::RDT_DOUBLE;
    return fd;
}
//多维数组
//inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, rectangle&)
//{
//    fd->field_data_type = st_field_t::tpRectangle;
//    fd->alignment = sizeof(coord_t);
//    return fd;
//}
//#ifdef USE_STD_STRING
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, std::string&)
{
    return fd->setStringType(st_field_t::RDT_stdStr);
}
//inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, std::wstring&)
//{
//    return fd->setWStringType(st_field_t::tpStdWString);
//}
//#endif
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, char const*&)
{
    return fd->setStringType(st_field_t::RDT_STR);
}
inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, char*&)
{
    return fd->setStringType(st_field_t::RDT_STR);
}

//inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, wchar_t const*&)
//{
//    return fd->setWStringType(st_field_t::tpWString);
//}
//inline rttiFieldDescriptor* setDescribeFieldType(rttiFieldDescriptor* fd, wchar_t*&)
//{
//    return fd->setWStringType(st_field_t::tpWString);
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//第二种实现方式
//发现第一种实现无法实现根据名字或存储随机读取
class CFieldDescIterator;//下面定义

//该类描述一个struct中所有的字段定义信息
//与fastdb每个类实例表示一个字段不同,该类实例包含struct中所有的字段信息
class CFieldDescriptor
{
  public:
    CFieldDescriptor();
   ~CFieldDescriptor();

    //不必实现拷贝构造函数, 编译器提供的默认行为已经足够;
    //CFieldDescriptor(const CFieldDescriptor &obj);

    //赋值函数 不必实现拷贝构造函数, 编译器提供的默认行为已经足够;
    //CFieldDescriptor &operator=(const CFieldDescriptor &obj);

  public:
    //功能: 添加一个字段定义
    //参数:
    //  [in] data_type: 字段类型
    //  [in] name: 字段名
    //  [in] comment: 字段说明
    //  [in] st_offset:struct结构中,字段相对于struct头偏移
    //  [in] offset: 字段相对于pack头偏移(用于序列化)
    //  [in] size:   字段大小
    //  [in] count:  字段个数,用于指定数组
    //返回值:
    //   0-成功;<0-失败
    int append_field(RTTI_DATA_TYPE data_type, char *name, char *comment,int st_offset, int offset, int size, int count);
    //功能: 添加一个字段定义, 偏移offset根据最后一个字段自动计算; 注:默认字节对齐方式; 暂时未考虑其他对齐方式
    int append_field(RTTI_DATA_TYPE data_type, char *name, char *comment, int st_offset, int size);

    ST_FIELD_DESC * add_field(char *name, char *comment, int st_offset, int size);
    
    //这两个函数暂时不需要
    //int del_field(int index);
    //int del_field(char *name);

    //增加不同类型的字段定义
    int add_char(char *name, char *comment, int st_offset)          { return append_field(RDT_CHAR,   name, comment, st_offset, sizeof(char)); }
    int add_uchar(char *name, char *comment, int st_offset)         { return append_field(RDT_UCHAR,  name, comment, st_offset, sizeof(char)); }
    int add_int2(char *name, char *comment, int st_offset)          { return append_field(RDT_INT2,   name, comment, st_offset, sizeof(short)); }
    int add_uint2(char *name, char *comment, int st_offset)         { return append_field(RDT_UINT2,  name, comment, st_offset, sizeof(short)); }
    int add_int4(char *name, char *comment, int st_offset)          { return append_field(RDT_INT4,   name, comment, st_offset, sizeof(int)); }
    int add_uint4(char *name, char *comment, int st_offset)         { return append_field(RDT_UINT4,  name, comment, st_offset, sizeof(int)); }
    int add_long(char *name, char *comment, int st_offset)          { return append_field(RDT_LONG,   name, comment, st_offset, sizeof(long)); }
    int add_double(char *name, char *comment, int st_offset)        { return append_field(RDT_DOUBLE, name, comment, st_offset, sizeof(double)); }
    int add_str(char *name, char *comment, int st_offset, int size) { return append_field(RDT_STR,    name, comment, st_offset, size); }
    int add_varstr(char *name, char *comment, int st_offset)        { return append_field(RDT_VAR_STR,name, comment, st_offset, 1024); }
    
    
  public:
    //返回字段个数
    int count() { return m_field_list.size(); }

    //获取序列化大小
    int get_serial_size()
    {
       int size=0;
       std::vector<ST_FIELD_DESC>::reverse_iterator iter;
       if((iter=m_field_list.rbegin()) != m_field_list.rend())
          size = iter->offset + iter->size;
       return size; 
    }
    //获取struct结构体大小
    int get_st_size()
    {
       int size=0;
       std::vector<ST_FIELD_DESC>::reverse_iterator iter;
       if((iter=m_field_list.rbegin()) != m_field_list.rend())
          size = iter->st_offset + iter->size;
       return size;
    }

    //功能:获取字段定义
    //参数:
    //  [in]index:字段索引
    //返回值:
    //   0-成功; <0-不存在;
    int                  find_field(int index, ST_FIELD_DESC *field_desc);
    ST_FIELD_DESC       &find_field(int index);//以引用方式返回
    const ST_FIELD_DESC *find_field_ptr(int index);//以指针方式返回

    //功能:获取字段定义
    //参数:
    //  [in]name:字段名
    //返回值:
    //  0-成功; <0-不存在;
    int                  find_field(char *name, ST_FIELD_DESC *field_desc);
    const ST_FIELD_DESC *find_field_ptr(char *name);//以指针方式返回
    
    void *convert_serial2struct(unsigned char *src, size_t src_size, unsigned char *dst, size_t dst_size);
    void *convert_struct2serial(unsigned char *src, size_t src_size, unsigned char *dst, size_t dst_size);
    
  private:
    std::vector<ST_FIELD_DESC> m_field_list;
    
  public://迭代器相关
    typedef  CFieldDescIterator iterator;
    //typedef CFieldDescriptor::CFieldDescIterator iterator;
    CFieldDescriptor::iterator begin();
    CFieldDescriptor::iterator end();
};

inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, char&)
{
    fd->field_data_type = st_field_t::RDT_CHAR;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, unsigned char&)
{
    fd->field_data_type = st_field_t::RDT_UCHAR;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, short&)
{
    fd->field_data_type = st_field_t::RDT_INT2;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, unsigned short&)
{
    fd->field_data_type = st_field_t::RDT_UINT2;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, int&)
{
    fd->field_data_type = st_field_t::RDT_INT4;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, unsigned int&)
{
    fd->field_data_type = st_field_t::RDT_UINT4;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, long&)
{
    fd->field_data_type = st_field_t::RDT_LONG;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, unsigned long&)
{
    fd->field_data_type = st_field_t::RDT_ULONG;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, float&)
{
    fd->field_data_type = st_field_t::RDT_FLOAT;
    return fd;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, double&)
{
    fd->field_data_type = st_field_t::RDT_DOUBLE;
    return fd;
}

inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, std::string&)
{
    return fd->field_data_type = st_field_t::RDT_stdStr;
}

inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, char const*&)
{
    return fd->field_data_type = st_field_t::RDT_STR;
}
inline ST_FIELD_DESC* setDescribeFieldType(ST_FIELD_DESC* fd, char*&)
{
    return fd->field_data_type = st_field_t::RDT_STR;
}

//迭代器元素指针
//template<typename T>
//class MxxIterNode
//{
//  public:
//    ItemNode() { m_nex=m_pre=NULL; }
//    T * get_ptr() const { return m_item; }
//    MxxItemNode * next() { return m_next; }
//    MxxItemNode * pre()  { return m_re; }
//  private:
//    T * m_item_ptr;
//    MxxItemNode *m_next;//下一个
//    MxxItemNode *m_pre;//上一个
//};

//--------------------------------------
////迭代器支持的属性:  traits表示特性的意思,故称为特性提取机(萃取机)
//// 如果想要与STL兼容,迭代器必须支持如下五种特性(最常用的); 另:必须为pointer和pointer-to-const设计特化版本
////   typedef tyepname T::iterator_category iterator_category; 
////   //STL支持五种迭代器类型,如下
////   //       std::input_iterator_tag/std::output_iterator_tag/std::forward_iterator_tag/std::bidirection_iterator_tag/std::random_access_iterator_tag;
////   typedef typename T::value_type        value_type;        //迭代器指向的数据类型
////   typedef typename T::difference_type   difference_type;   //两个迭代器之间的距离,故可以用来表示容器的最大容量
////   typedef typename T::pointer           pointer;
////   typedef typename T::reference         reference;
////  
////    value_type: 迭代器指向的数据类型
////  STL已经实现,见<stl_iterator.h>
//template<typenames __mxx_iterator>
//struct Mxxiterator_traits
//{
//    typedef typename __mxx_iterator::value_type          value_type;//类型
//    typedef typename __mxx_iterator::difference_type     difference_type;//??
//    typedef typename __mxx_iterator::pointer             pointer;    //指针*
//    typedef typename __mxx_iterator::reference          reference;   //引用
//    typedef typename __mxx_iterator::iterator_category  iterator_category;
//};
//
//// specialize for __mxx_iterator *
//template<typename __mxx_iterator>
//struct Mxxiterator_traits<__mxx_iterator *> //指明该模板仅适用与__mxx_iterator *
//{
//    typedef __mxx_iterator       value_type;//类型
//    typedef ptrdiff_t            difference_type;//??
//    typedef __mxx_iterator*       pointer;//指针*
//    typedef __mxx_iterator&       reference;//引用&
//    typedef random_access_iterator iterator_category;//原生指针是一种random_access_iterator
//};
//
//// specialize for const __mxx_iterator*
//template<typename __mxx_iterator>
//struct Mxxiterator_traits<const __mxx_iterator *>
//{
//    typedef __mxx_iterator       value_type;//类型                     
//    typedef ptrdiff_t            difference_type;//??
//    typedef const __mxx_iterator*       pointer;//指针*
//    typedef const __mxx_iterator&       reference;//引用&
//    typedef random_access_iterator iterator_category;
//};
//
//---------------------------------------

////迭代器样本
//template<typename mxxItem>
//class MxxIterator: public std::iterator<std::random_access_iterator_tag, mxxItem>
//{
//  //迭代器五个属性已经在弗雷中定义不需要重新定义;
//  public:
//    MxxIterator(mxxItem *ptr=NULL) : m_ptr(ptr) {}
//    ~MxxIterator(){}
//
//    //不必实现copy construct,编译器提供的缺省行为已经足够
//    //不必实现operator=,编译器提供的缺省行为已经足够
//  public:
//    mxxItem & operator*() const { return *m_ptr; }
//    mxxItem *operator->() const { return m_ptr;  }
//
//    //以下两个operator++遵循标准做法;
//    //pre++
//    MxxIterator & operator++() { ++m_ptr; return *this; }
//    //post++
//    MxxIterator oprator++(int) { MxxIterator tmp = *this; ++*this; return tmp; }
//
//   //pre--
//    MxxIterator & operator--() { --m_ptr; return *this; }
//    //post--
//    MxxIterator oprator++(int) { MxxIterator tmp = *this; --*this; return tmp; }
//
//    bool operator==(const MxxIterator &obj) const
//    {
//       if(this==&obj) return true;
//       return m_ptr == obj.m_ptr;
//    }
//    bool operator!=(const MxxIterator &obj) const { return m_ptr!=obj.m_ptr; }
//};

////迭代器
////template<class mxxItem>
//class MxxIterator: public std::iterator<std::random_access_iterator_tag, ST_FIELD_DESC>//兼容STL,故从std::iterator继承
//{
//  //迭代器五个属性已经在父类中定义,不需要重新定义;
//
//  public:
//    MxxIterator(ST_FIELD *elem_ptr=NULL, CFieldDescriptor *container_ptr=NULL);
//    ~MxxIterator();
//
//    //不必实现copy construct,编译器提供的缺省行为已经足够
//    //不必实现operator=,编译器提供的缺省行为已经足够
//  public:
//    mxxItem & operator*() const;
//    mxxItem * operator->() const; 
//    
//    //以下两个operator++遵循标准做法;
//    //pre++
//    MxxIterator & operator++(); 
//    //post++
//    MxxIterator operator++(int);
//
//    MxxIterator &operator--();
//    MxxIterator operator--(int);
//
//    bool operator==(const MxxIterator &obj) const; 
//   
//    bool operator!=(const MxxIterator &obj);
//  private:
//    CFieldDescriptor  *m_container_ptr;
//    int m_cursor;//当前字段索引
//};


//////////////////////////////////////////////////////////迭代器实现///////////////////////////////////////////////////////////

//迭代器
//I'd known if i can define a iterator which is not template class and derived from std::iterator
//but accroding to testing result, it seem to work correnctly.
class CFieldDescIterator: public std::iterator<std::random_access_iterator_tag, ST_FIELD_DESC>//兼容STL,故从std::iterator继承
{
  //迭代器五个属性已经在父类中定义,不需要重新定义;

  public:
    CFieldDescIterator(ST_FIELD_DESC *elem_ptr=NULL,CFieldDescriptor *container_ptr=NULL) : m_container_ptr(container_ptr)
    {
      //m_elem_ptr=NULL;//不用设置元素指针了
      m_container_ptr=container_ptr;//容器指针,暂时这么实现吧！
      m_cursor=0;//当前索引=0;
    }
    ~CFieldDescIterator(){}

    //不必实现copy construct,编译器提供的缺省行为已经足够
    //不必实现operator=,编译器提供的缺省行为已经足够
  public:
    ST_FIELD_DESC & operator*() const
    {
        ST_FIELD_DESC *ptr=NULL;

       if( (m_cursor>=0) && (NULL!=m_container_ptr))
       {
          //return m_container_ptr->get_field(m_cursor);
          ptr=m_container_ptr->get_field_ptr(m_cursor);
       }
       return *ptr;//NULL will result in core dump
    };
    ST_FIELD_DESC * operator->() const
    {
       if( (m_cursor>=0) && (NULL!=m_container_ptr))
       {
          return m_container_ptr->get_field_ptr(m_cursor);
       }
       return NULL;
    }

    //以下两个operator++遵循标准做法;
    //pre++
    CFieldDescIterator & operator++()
    {
       ++m_cursor;
      if(m_cursor>=m_container_ptr->count())//到达结尾了!
         m_cursor=-1;
      return *this;
    }
    //post++
    CFieldDescIterator operator++(int) { CFieldDescIterator tmp = *this; ++*this; return tmp; }
    //pre--
    CFieldDescIterator &operator--()
    {
      --m_cursor;
      return *this;
    }
    //post--
    CFieldDescIterator operator--(int){ CFieldDescIterator tmp = *this; --*this; return tmp;}

    bool operator==(const CFieldDescIterator &obj) const
    {
       if(this==&obj) return true;
       return (m_cursor== obj.m_cursor) && (m_container_ptr==obj.m_container_ptr);
    }

    bool operator!=(const CFieldDescIterator &obj)
    {
       return (m_cursor!= obj.m_cursor) || (m_container_ptr!=obj.m_container_ptr);
    }

  public: //特有函数
    void set_cursor(int index) { m_cursor = index; }
  private:
    CFieldDescriptor  *m_container_ptr;
    int m_cursor;//当前字段索引
};

//////////////////////////////////////////迭代器实现完成////////////////////////////////////////////////

#endif
