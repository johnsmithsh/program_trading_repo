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

#define mxx_varmem_truncate(mem_ptr) mxx_varmem_clear(mem_ptr)

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

#define EVARMEM_OutOfMemory  -101 //缓存溢出
/* 
 * @brief 从头开始写数据
 * @param
 *   [in]mem_ptr:
 *   [in]data_ptr,data_len: 写入数据的指针和长度;
 * @retVal 返回写入字节数; <0-失败; 
 *   错误码: EVARMEM_OutOfMemory
*/
inline int mxx_varmem_assign(mxx_varmem_t *mem_ptr, unsigned char *data_ptr, size_t data_len)
{
    if((data_len<=0)||(NULL==data_ptr))
        return 0;
    if(data_len>=mxx_varmem_availsize(mem_ptr))//!<缓存长度不足
        return EVARMEM_OutOfMemory;
    
    unsigned char *data_address=mxx_varmem_address(mem_ptr);
    memcpy(data_address, data_ptr, data_len);
    mem_ptr->membuff.length = data_len;
    return mem_ptr->membuff.length;
}

/*
 * @brief 在指定的位置写入数据; startpos范围[0,buffsize-1];
 * @param
 *   [in]mem_ptr:
 *   [in]startpos: 范围[0, buffsize-1]
 *   [in]data_ptr,data_len: 写入数据的指针和长度;
 * @retVal 返回写入字节数; <0-失败; 
 *   错误码:EVARMEM_OutOfMemory
*/
inline int mxx_varmem_write(mxx_varmem_t *mem_ptr, uint32_t startpos, unsigned char *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0)) 
        return 0;
    if(startpos>=mxx_varmem_availsize(mem_ptr))//!<缓存长度不足
        return EVARMEM_OutOfMemory;
    if(startpos+data_len>mxx_varmem_availsize(mem_ptr))//可以==
        data_len = mxx_varmem_availsize(mem_ptr) - startpos;//!< size_t一定大于0,故必须判断buffsize>startpos;
    if(data_len<=0)
        return 0;
    
    //拷贝数据
    unsigned char *dst=mxx_varmem_address(mem_ptr)+startpos;
    memcpy(dst, data_ptr, data_len);

    mem_ptr->membuff.length += data_len;

    return data_len;
}

/*
 * @brief 追加数据; 0-成功; <0-失败;
 * @param
 *    [in]mem_ptr:
 *    [in]data_ptr,data_len: 写入数据的指针和长度;
 * @retVal 返回写入字节数; <0-失败; 
 *   错误码:EVARMEM_OutOfMemory
 */
inline int mxx_varmem_append(mxx_varmem_t *mem_ptr, unsigned char *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0)) 
        return 0;

    if(mxx_varmem_availsize(mem_ptr)<data_len)//!<缓存长度不足
        return EVARMEM_OutOfMemory;
    memcpy(mem_ptr->membuff.databuff+mem_ptr->membuff.length, data_ptr, data_len);
    mem_ptr->membuff.length += data_len;
    return data_len;
}

/*
 * @brief 插入数据; startpos从0开始; 如果指定位置在最后位置之后,则变为追加;
 *   [in]startpos: 范围[0, length-1];>=length,则从length追加
 *   [in]data_ptr,data_len: 写入数据的指针和长度;
 * @retVal
 *  返回字节数; <0-失败;
 */
inline int mxx_varmem_insert(mxx_varmem_t *mem_ptr, uint32_t startpos, unsigned char *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0)) 
        return 0;
    if(mxx_varmem_availsize(mem_ptr)<data_len)//!<缓存长度不足
        return EVARMEM_OutOfMemory;
    if(startpos>mxx_varmem_datalen(mem_ptr))//可以==,等同于追加
        startpos = mxx_varmem_datalen(mem_ptr);
    //后面所有元素后移动
    unsigned char *src=mxx_varmem_address(mem_ptr)+mxx_varmem_datalen(mem_ptr)-1;
    unsigned char *dst=src+data_len;
    int count = mxx_varmem_datalen(mem_ptr) - startpos;
    while(count-->0)
        *dst-- = *src--;

    //插入内容
    dst=mxx_varmem_address(mem_ptr)+startpos;
    memcpy(dst, data_ptr, data_len);

    mem_ptr->membuff.length += data_len;
    //*(data_ptr+mem_ptr->membuff.length) = 0;

    return data_len;
}

/* 
 * @brief 填充数据
 * @param
 *   [in]mem_ptr
 *   [in]startpos: 范围[0, length-1];>=length,则从length开始填充
 *   [in]ch,data_len: 写入数据的指针和长度;
 * @retVal
 *  返回字节数; <0-失败;
 */
inline int mxx_varmem_fill(mxx_varmem_t *mem_ptr, uint32_t startpos, unsigned char ch,  size_t len)
{
    if(len<=0)
        return 0;//return -1;
    //起始位置必须在0~data_len之间,否则从数据结尾开始填充
    if(startpos>mxx_varmem_datalen(mem_ptr))
        startpos = mxx_varmem_datalen(mem_ptr);
    //填充个数不能超出缓存区大小
    if(startpos+len>mxx_varmem_buffsize(mem_ptr))
        len = mxx_varmem_buffsize(mem_ptr) - startpos;
    memset(mem_ptr->membuff.databuff+startpos, ch, len);

    if(startpos+len>mxx_varmem_datalen(mem_ptr))
        mem_ptr->membuff.length = startpos+len;
    return len;//return  mem_ptr->membuff.length;
}

/*
 * @brief 删除数据
 * @param
 *   [in]startpos: 范围[0, length-1];>=length,不删除数据;
 *   [in]data_len: 删除数据长度;
 */
inline mxx_varmem_t *mxx_varmem_erase(mxx_varmem_t *mem_ptr, uint32_t startpos, size_t len)
{
    //起始位置在0~length-1之外,则不需要处理
    if(startpos>=mxx_varmem_datalen(mem_ptr))
        return mem_ptr;
    
    //必须保证startpos~startpos+len,在0~length之间
    if(mxx_varmem_datalen(mem_ptr)-startpos<len)
        len = mxx_varmem_datalen(mem_ptr)-startpos;
    if(len<=0)
        return mem_ptr;
    
    //后面元素前移
    unsigned char *begin=mxx_varmem_address(mem_ptr);
    unsigned char *src  =begin + startpos+len;
    unsigned char *dst  =begin + startpos;
    int          count  =mxx_varmem_datalen(mem_ptr) - startpos-len;
    if(count>0)//<=0说明从起始位置删除到尾部
        memmove(dst, src, count);

    mem_ptr->membuff.length -= len;//!<长度发生变化
    *(begin+mem_ptr->membuff.length) = 0;

    return mem_ptr;
}

/*
 * @brief 替换数据
 * @param
 *   [in]startpos: 范围[0, length-1];>=length,不做处理;
 *   [in]data_ptr,data_len: 替换数据长度;
 * @retVal
 *   返回替换的字节数;
 */
inline int mxx_varmem_replace(mxx_varmem_t *mem_ptr, uint32_t startpos, unsigned char *data_ptr, size_t data_len)
{
    if((NULL==data_ptr)||(data_len<=0))
        return 0;//return -1;
    //起始位置在0~length-1之外,则不需要处理
    if(startpos>=mxx_varmem_datalen(mem_ptr))
        return 0;//return -1;
    
    //替换长度不能大于缓存
    if(startpos+data_len>mxx_varmem_buffsize(mem_ptr))
        data_len = mxx_varmem_buffsize(mem_ptr) - startpos;
    memcpy(mem_ptr->membuff.databuff+startpos, data_ptr, data_len);

    //可能影响数据长度
    if(startpos+data_len>mxx_varmem_datalen(mem_ptr))
        mem_ptr->membuff.length = data_len+startpos;

    return data_len;//return  mem_ptr->membuff.length;
}

/*
 * @brief 跳过指定长度; 返回数据长度
 * @param
 *   [in]startpos: 范围[0, length-1];>=length,不做处理;
 *   [in]data_ptr,data_len: 替换数据长度;
 * @retVal
 *   返回替换的字节数;
 */
inline int mxx_varmem_skip(mxx_varmem_t *mem_ptr, size_t len)
{
    if(len<=0)
        return 0;
    if(mem_ptr->membuff.length+len>mem_ptr->size)
        len = mem_ptr->size - mem_ptr->membuff.length;
    mem_ptr->membuff.length += len;
    return len;
}

#endif
