rttiFieldDescriptor::rttiFieldDescriptor(const char *field_name, size_t offset, size_t size, int index)
{
    memset(&m_field, 0, sizeof(m_field));
    m_field.prev=m_field.next=this;//构成环
    m_field.field_index=index;
    strncpy(m_field.field_name, field_name, sizeof(m_field.field_name)-1);
    
    m_field.field_data_type = st_field_t::RDT_UNKONOWN;//字段类型
  
    m_field.count        = 1;
    m_field.st_offset    = offset;   //字段在struct中的偏移(struct定义) 
    m_field.proto_offset = 0;        //字段在序列流中的偏移(用于结构体序列化)
    m_field.field_size   = size;     //字段大小;
}

rttiFieldDescriptor::~rttiFieldDescriptor()
{
}

//@brief 链表操作: 根据字段名找到对应的描述信息
rttiFieldDescriptor *rttiFieldDescriptor::find_field(const char *name)
{
    if((NULL==name)||('\0'==*name))
        return NULL;
    
    rttiFieldDescriptor *rfd=this;
    st_field_t *pField=NULL;
    do
    {
        pField = rfd->get_field_info();
        //防止数据结构发生变化,加载旧结构数据出现问题,故必须判断大小;
        if(0==strcmp(name, pField->field_name))
            return rfd;
    }
    while((rfd=rfd->next) != this);
    
    return NULL;
}

//@brief 链表操作: 根据索引找到对应的描述信息
rttiFieldDescriptor *find_field(int index)
{
    rttiFieldDescriptor *rfd=this;
    st_field_t *pField=NULL;
    do
    {
        pField = rfd->get_field_info();
        //防止数据结构发生变化,加载旧结构数据出现问题,故必须判断大小;
        if(index==pField->field_index)
            return rfd;
    }
    while((rfd=rfd->next) != this);
    
    return NULL;
}

//@brief 链表操作: serial数据转换为应用程序使用的struct数据
void *rttiFieldDescriptor::convert_serial2struct(unsigned char *src, size_t src_size, unsigned char *dst, size_t dst_size)
{
    if((NULL==src)||(dst==NULL))
        return NULL;
    if((src_size<=0)||(dst_size<=0))
        return NULL;
    
    rttiFieldDescriptor *rfd=this;
    st_field_t *pField=NULL;
    do
    {
        pField = rfd->get_field_info();
        //防止数据结构发生变化,加载旧结构数据出现问题,故必须判断大小;
        if((pField->st_offset+pField->field_size<=src_size)&&(pField->st_offset+pField->field_size<=dst_size))
            memcpy(dst+pField->st_offset, src+pField->proto_offset, pField->field_size);
    }
    while((rfd=rfd->next) != this);
    
    return dst;
}

//@brief 链表操作: struct数据转换为serial数据
void *rttiFieldDescriptor::convert_struct2serial(unsigned char *src, size_t src_size, unsigned char *dst, size_t dst_size)
{
    if((NULL==src)||(dst==NULL))
        return NULL;
    if((src_size<=0)||(dst_size<=0))
        return NULL;
    
    rttiFieldDescriptor *rfd=this;
    st_field_t *pField=NULL;
    do
    {
        pField = rfd->get_field_info();
        //防止数据结构发生变化,加载旧结构数据出现问题,故必须判断大小;
        if((pField->st_offset+pField->field_size<=src_size)&&(pField->st_offset+pField->field_size<=dst_size))
            memcpy(dst+pField->proto_offset, src+pField->st_offset, pField->field_size);
    }
    while((rfd=rfd->next) != this);
    
    return dst;
}

//@brief 链表操作: 清除struct数据
void rttiFieldDescriptor::clear_struct(unsigned char *dst, size_t dst_size)
{
    if((dst==NULL))
        return ;
    if((dst_size<=0))
        return ;
    
    rttiFieldDescriptor *rfd=this;
    st_field_t *pField=NULL;
    do
    {
        pField = rfd->get_field_info();
        //防止数据结构发生变化,加载旧结构数据出现问题,故必须判断大小;
        if(pField->st_offset+pField->field_size<=dst_size))
            memset(dst+pField->st_offset, 0, pField->field_size);
    }
    while((rfd=rfd->next) != this);
    
    return;
}