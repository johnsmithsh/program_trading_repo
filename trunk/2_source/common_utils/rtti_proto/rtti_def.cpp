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
    m_field.size         = size;     //字段大小;
}

rttiFieldDescriptor::~rttiFieldDescriptor()
{
}