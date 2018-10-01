#ifndef __MXX_MEM_BUFFER_H_
#define __MXX_MEM_BUFFER_H_

#include <stdint.h>
#include <stddef.h>

#include <string.h>
#include <stdlib.h>

//@brief 缓存数据
typedef struct _st_mxx_mem
{
    uint32_t length;           //!< 数据长度(注:不是缓存大小)
    unsigned char databuff[0]; //!< 数据指针
} mxx_mem_t;

//@breif 缓存
typedef struct _st_mxx_varmem
{
    uint32_t  size;   //!< 缓存大小; 注意: 这个不能随意更改;
    mxx_mem_t membuff;//!< 数据, 包含数据长度和数据
} mxx_varmem_t;

//typedef struct __st_mxx_varmem2
//{
//  uint32_t  size;   //!< 缓存大小; 注意: 这个不能随意更改;
//  uint32_t  length;  //!< 数据长度(注:不是缓存大小)
//  unsigned char *mem_ptr;
//}mxx_varmem2_t;

//@brief 创建缓存, 参数size缓存大小;实际缓存大小
// NULL 创建失败
inline mxx_varmem_t * mxx_varmem_create(size_t size)
{
    if(size<=0)
        return NULL;
    mxx_varmem_t *mem_ptr = (mxx_varmem_t *)malloc(size+sizeof(mxx_varmem_t));
    if(NULL==mem_ptr)
        return NULL;
    mem_ptr->size           = size;
    mem_ptr->membuff.length = 0;
    memset(mem_ptr->membuff.databuff, 0, size);

    return mem_ptr;
}

//@brief 将已有缓存转换为mxx_varmem_t; 注: size+sizeof(mxx_varmem_t)必须保证在合法范围内;大小必须大于
inline mxx_varmem_t * mxx_mem_to_varmem(unsigned char *buff_ptr, size_t size)
{
    mxx_varmem_t *mem_ptr = (mxx_varmem_t *)buff_ptr;
    if(NULL==mem_ptr)
        return NULL;
    mem_ptr->size = size - sizeof(mxx_varmem_t);
    return mem_ptr;
}

//@brief 删除缓存
inline void mxx_varmem_destroy(mxx_varmem_t * &mem_ptr)
{
    if(NULL==mem_ptr) 
        return;
    free(mem_ptr);
    mem_ptr=NULL;
}

//@brief 清理数据
inline void mxx_varmem_clear(mxx_varmem_t *mem_ptr)
{
    mem_ptr->membuff.length = 0;
    memset(mem_ptr->membuff.databuff, 0, mem_ptr->size);
}

//@brief 获取缓存地址
inline unsigned char *mxx_varmem_address(mxx_varmem_t *mem_ptr)
{
    return mem_ptr->membuff.databuff;
}

//@brief 缓存空闲地址
inline unsigned char *mxx_varmem_freeaddress(mxx_varmem_t *mem_ptr)
{
    return (mem_ptr->size > mem_ptr->membuff.length) ? mem_ptr->membuff.databuff + mem_ptr->membuff.length : NULL;
}

//@brief 可用缓存大小; 缓存大小-数据长度
inline size_t mxx_varmem_availsize(mxx_varmem_t *mem_ptr)
{
    return (mem_ptr->size > mem_ptr->membuff.length) ? mem_ptr->size - mem_ptr->membuff.length : 0;
}

//@brief 缓存大小
inline size_t mxx_varmem_buffsize(mxx_varmem_t *mem_ptr)
{
    return mem_ptr->size;
}

//@brief 数据长度
inline size_t mxx_varmem_datalen(mxx_varmem_t *mem_ptr)
{
    return mem_ptr->membuff.length;
}

//@brief 设置数据
inline int mxx_varmem_assign(mxx_varmem_t *mem_ptr, unsigned char *data_ptr, size_t data_len)
{
    if((data_len<=0)||(NULL==data_ptr))
        return 0;
    if(data_len>mem_ptr->size)
        return -1;
    
    unsigned char *data_address=mxx_varmem_address(mem_ptr);
    memcpy(data_address, data_ptr, data_len);
    mem_ptr->membuff.length = data_len;
    return 0;
}

//@brief 追加数据; 0-成功; <0-失败;
inline int mxx_varmem_append(mxx_varmem_t *mem_ptr, unsigned char *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0)) 
        return 0;

    if(mxx_varmem_availsize(mem_ptr)<data_len)//!<缓存长度不足
        return -1;
    memcpy(mem_ptr->membuff.databuff+mem_ptr->membuff.length, data_ptr, data_len);
    mem_ptr->membuff.length += data_len;
    return 0;
}

//@brief 插入数据; startpos从0开始; 如果指定位置在最后位置之后,则变为追加;
//@retVal: 0-成功; <0-失败;
inline int mxx_varmem_insert(mxx_varmem_t *mem_ptr, uint32_t startpos, unsigned char *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0)) 
        return 0;
    if(mxx_varmem_availsize(mem_ptr)<data_len)//!<缓存长度不足
        return -1;
    if(startpos>mxx_varmem_datalen(mem_ptr))
        startpos = mxx_varmem_datalen(mem_ptr);
    //后面所有元素后移动
    unsigned char *src=mxx_varmem_address(mem_ptr)+mxx_varmem_datalen(mem_ptr)-1;
    unsigned char *dst=src+data_len;
    int count = mxx_varmem_datalen(mem_ptr) - startpos;
    while(count-->0)
        *dst-- = *src--;
    //unsigned char *data_address=mxx_varmem_address(mem_ptr)+startpos;
    //unsigned char *ptr=data_ptr + mem_ptr->membuff.length;
    //for(int index = mem_ptr->membuff.length-1; index>=startpos; --index)
    //{
    //    *(ptr+data_len) = *(data_address+index);
    //    --ptr;
    //}

    //插入内容
    dst=mxx_varmem_address(mem_ptr)+startpos;
    memcpy(dst, data_ptr, data_len);

    mem_ptr->membuff.length += data_len;
    //*(data_ptr+mem_ptr->membuff.length) = 0;

    return 0;
}

//@brief 填充数据
inline int mxx_varmem_fill(mxx_varmem_t *mem_ptr, unsigned char ch, size_t len, uint32_t startpos=0)
{
    if(startpos+len>mxx_varmem_buffsize(mem_ptr))
        len = mxx_varmem_buffsize(mem_ptr) - startpos;
    memset(mem_ptr->membuff.databuff+startpos, ch, len);

    if(len+startpos>mxx_varmem_datalen(mem_ptr))
            mem_ptr->membuff.length = len+startpos;
    return  mem_ptr->membuff.length;
}

//@brief 删除数据
inline mxx_varmem_t *mxx_varmem_erase(mxx_varmem_t *mem_ptr, uint32_t startpos=0, size_t len=0)
{
    if(mem_ptr->membuff.length-startpos<len)
        len = mem_ptr->membuff.length-startpos;
    //后面元素前移
    unsigned char *data_address=mxx_varmem_address(mem_ptr);
    unsigned char *dst=data_address + startpos;
    int count=0;
    for(int index = startpos + len; (count<len)&&(index<mem_ptr->membuff.length); ++index)
    {
        *dst = *(data_address+index);
        ++dst;
        ++count;
    }

    mem_ptr->membuff.length -= len;
    *(data_address+mem_ptr->membuff.length) = 0;

    return mem_ptr;
}

//@brief 替换数据
inline int mxx_varmem_replace(mxx_varmem_t *mem_ptr, unsigned char *data_ptr, size_t data_len, uint32_t startpos=0)
{
    if(startpos>=mxx_varmem_datalen(mem_ptr))
        return 0;

    if(startpos+data_len>mxx_varmem_buffsize(mem_ptr))
        data_len = mxx_varmem_buffsize(mem_ptr) - startpos;
    memcpy(mem_ptr->membuff.databuff+startpos, data_ptr, data_len);

    if(data_len+startpos>mxx_varmem_datalen(mem_ptr))
        mem_ptr->membuff.length = data_len+startpos;

    return  mem_ptr->membuff.length;
}

//@brief 跳过指定长度; 返回数据长度
inline int mxx_varmem_skip(mxx_varmem_t *mem_ptr, size_t len)
{
    mem_ptr->membuff.length += len;
    if(mem_ptr->membuff.length>mem_ptr->size)
        mem_ptr->membuff.length = mem_ptr->size;
    return mem_ptr->membuff.length;
}

#endif
