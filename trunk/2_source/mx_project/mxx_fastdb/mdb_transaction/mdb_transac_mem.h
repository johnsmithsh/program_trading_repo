#ifndef __MDB_TRANSAC_MEM_H_
#define __MDB_TRANSAC_MEM_H_
/*
 * 内存库事务处理过程中,发现内存管理类似队列,且不会从中间删除和插入
 * 为了提高性能,将事务内存管理单独处理;
 * 级别:
 *  mempool_t->memblock_t(多个memunit_info_t)
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
 
//@brief 表示内存池的一个内存块
typedef struct __st_memblock
{
    void *buff_ptr; //内存指针
    unsigned int buff_size;//内存大小
    unsigned int used_size;//已使用大小
    //int idle_size;//空闲大小
    //st_mempool *owner_pool;//属主
    
    struct __st_memblock *prev;//上一个
    struct __st_memblock *next;//下一个
} st_memblock;

//@brief 创建一个新的memblock; NULL表示堆申请内存失败
inline st_memblock * memblock_create(size_t block_size)
{
    assert(block_size>=sizeof(st_memblock));
    if(block_size < sizeof(st_memblock))
    {
        block_size=sizeof(st_memblock);
    }
    st_memblock *block_ptr=(st_memblock *)malloc(block_size);
    if(NULL==block_ptr)
        return NULL;
    memset(block_ptr, 0, sizeof(st_memblock));
    block_ptr->buff_ptr  = (void *)block_ptr+sizeof(st_memblock);
    block_ptr->buff_size = block_size-sizeof(st_memblock);
    block_ptr->used_size = 0;
    block_ptr->prev      = NULL;
    block_ptr->next      = NULL;
    return block_ptr;
}

//@brief 功能: 删除memblock,并发返回next
inline st_memblock * memblock_delete(st_memblock *memblock)
{
    assert(NULL!=memblock);
    
    //防止该内存块还在内存链表中
    st_memblock *tmp_block_prev=memblock->prev;
    st_memblock *tmp_block_next=memblock->next;
    if(NULL!=tmp_block_prev)
        tmp_block_prev->next = tmp_block_next;
    if(NULL!=tmp_block_next)
        tmp_block_next->prev = tmp_block_prev;
    
    st_memblock *next_ptr = memblock->next;
    free(memblock); //释放内存
    return next_ptr;
}

//@brief功能: 计算内存内存块空闲内存大小
inline unsigned int memblock_idlesize(st_memblock *memblock)
{
    return memblock->buff_size-memblock->used_size;
}

//@brief 功能: 分配内存; NULL-表示内存块空闲内存不足;
//@note 此处分配的内存没有进行对齐处理,分配的空间可能影响存取效率;
inline void * memblock_alloc(st_memblock *memblock, size_t alloc_size)
{
    assert(alloc_size>0);
    if(memblock_idlesize(memblock)<alloc_size)
        return NULL;
    void *ptr=memblock->buff_ptr+memblock->used_size;
    memset(ptr, 0, alloc_size);
    memblock->used_size += alloc_size;
    return ptr;
}

//@brief 功能: 释放被占用的内存;注:从后向前释放指定大小; 
inline void  memblock_free(st_memblock *memblock, size_t free_size)
{
    assert(free_size>0);
    if(memblock->used_size<free_size)
        memblock->used_size=0;
    else
        memblock->used_size -= free_size;
    return ;
}

//@brief 功能:释放内存块中被占用的所有内存
inline void memblock_free(st_memblock *memblock)
{
    memblock_free(memblock, memblock->used_size);
}

//typedef struct __st_memunit_info
//{
//   st_memblock *owner_block;//属主
//   void *next;//下一块内存地址
//}st_memunit_info;

//@brief 功能:队列中插入一个memblock; 返回src;
inline st_memblock *mempool_insert(st_memblock *dst, st_memblock *src, bool b_after=true)
{
    assert(NULL!=src);
    //assert(NULL!=dst);
    
    st_memblock *tmp_block_ptr=NULL;
    if(b_after)
    {
        if(NULL!=dst)
        {
          tmp_block_ptr=dst->next;
          dst->next = src;
        }
        src->prev = dst;
        src->next =tmp_block_ptr;
        if(NULL!=tmp_block_ptr)
            tmp_block_ptr->prev = src;
    }
    else
    {
        if(NULL!=dst)
        {
          tmp_block_ptr=dst->prev;
          dst->prev=src;
        }
        src->prev = tmp_block_ptr;
        src->next = dst;
        if(NULL!=tmp_block_ptr)
            tmp_block_ptr->next = src;            
    }
    
    return src;
}

//@brief 链表dst追加一个memblock; 返回src;
inline st_memblock *mempool_append(st_memblock *dst, st_memblock *src)
{
    if(NULL==dst)
    {
        src->prev = dst;
        src->next = NULL;
        return src;
    }
    while(NULL!=dst->next) dst = dst->next;
    
    mempool_insert(dst, src, true);
    
    return src;    
}
//功能:队列中移除一个memblock; 返回被移除的memblock,即入参;
inline st_memblock *mempool_remove(st_memblock *memblock)
{
    if(NULL==memblock) 
        return NULL;
    
    st_memblock *tmp_block_prev=memblock->prev;
    st_memblock *tmp_block_next=memblock->next;
    if(NULL!=tmp_block_prev)
        tmp_block_prev->next = tmp_block_next;
    if(NULL!=tmp_block_next)
        tmp_block_next->prev = tmp_block_prev;
    
    memblock->prev = memblock->next = NULL;
    return tmp_block_next;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MEM_ALIGN_SIZE (1<<2) //默认内存对齐方式
//将mem_size大小转为内存对齐; mem_size-内存大小; align_size对齐方式; 1,2、4、8等2的指数;
inline unsigned int to_memalign(unsigned int mem_size, unsigned int align_size=MEM_ALIGN_SIZE)
{
    return (mem_size+align_size-1)&(~(align_size-1));
}

#define MEMPOOL_MIN_SIZE    32   //内存池最小大小
#define MEMPOOL_INIT_SIZE   4096 //内存池初始大小
#define MEMPOOL_GROW_SIZE   1024 //每次增长大小
//内存管理
class CTransacMempool
{
  public:
    //init_size不能小于sizeof(st_memblock)
    CTransacMempool(unsigned int init_size=MEMPOOL_INIT_SIZE)
    {
       
      #ifdef SINGLE_UNITTEST //单元测试
        m_init_size = m_grow_size = 0;
      #else
        m_init_size = init_size;
        if(init_size<MEMPOOL_MIN_SIZE)
            m_init_size = MEMPOOL_MIN_SIZE;
        
        m_grow_size = MEMPOOL_GROW_SIZE;
      #endif
        
        m_head_ptr = m_tail_ptr = NULL;
    }
    virtual ~CTransacMempool()
    {
        st_memblock *memblock_ptr=m_head_ptr;
        st_memblock *tmp_blockptr=NULL;
        //删除链表中各个内存块占用的堆内存
        while(NULL!=memblock_ptr)
        {
            tmp_blockptr = memblock_ptr->next;
            memblock_delete(memblock_ptr);//删除内存块
            memblock_ptr = tmp_blockptr;
        }
        m_head_ptr = m_tail_ptr = NULL;
    }
  private:
    //禁用拷贝构造函数
    CTransacMempool(const CTransacMempool &obj);
    //禁用赋值函数
    CTransacMempool &operator=(const CTransacMempool &obj);
  public:
  
    //@brief 从内存池分配空间;
    void *alloc(size_t alloc_size)
    {
        assert(alloc_size>0);
        if(!check_idle_size(alloc_size))
        {
            if(!append_memblock(alloc_size))
                return NULL;
        }
        
        //从内存块分配空间,空间此时一定充足
        void *data_ptr = memblock_alloc(m_tail_ptr, alloc_size);
        return data_ptr;
    }
    //@brief 释放内存池所有被占用的资源;(不释放内存)
    void free()
    {
        st_memblock *memblock_ptr=NULL;
        for(memblock_ptr=m_head_ptr; memblock_ptr!=NULL; memblock_ptr=memblock_ptr->next)
        {
            memblock_free(memblock_ptr);
        }
        
        m_tail_ptr = m_head_ptr;//此处为了内存池可重复使用
    }
  private:
    //@brief 校验剩余空间大小; true-剩余空间充足; false-剩余空间不足;
    bool check_idle_size(unsigned int alloc_size)
    {
        if(NULL==m_tail_ptr)
            return false;
        
        //找到满足大小要求的内存块
        st_memblock *memblock_ptr = NULL;
        for(memblock_ptr = m_tail_ptr; NULL!=memblock_ptr; memblock_ptr = memblock_ptr->next)
        {
            m_tail_ptr=memblock_ptr;//链表尾部指针后移; 需要防止本次申请内存大于链表内所有内存块的可用空间
            
            if(memblock_idlesize(memblock_ptr)>=(size_t)alloc_size)//该内存块空间满足大小要求
                return true;
        }
        
        return false;
    }
    
    //@brief 增加新的内存块
    bool append_memblock(int alloc_size)
    {
        unsigned int grow_size=sizeof(st_memblock)+alloc_size;
        if(NULL==m_tail_ptr)//第一次分配
        {
            if(grow_size<m_init_size)
                grow_size=m_init_size;
        }
        else //以后每次增长,增加的内存卡都是max(2*上一内存块大小, 本次申请内存大小)
        {
            m_grow_size = sizeof(st_memblock)+2*m_tail_ptr->buff_size;
            if(grow_size<m_grow_size)
                grow_size = m_grow_size;
        }
        
        grow_size=to_memalign(grow_size, 1);//内存对齐
            
        //申请新内存块
        st_memblock *memblock_ptr = memblock_create(grow_size);
        if(NULL==memblock_ptr)
            return false;
        
        //添加到队列中
        if(NULL==m_tail_ptr)//第一次申请空间
        {
            m_tail_ptr=m_head_ptr=memblock_ptr;
        }
        else
        {
            mempool_insert(m_tail_ptr, memblock_ptr, true);
            m_tail_ptr = memblock_ptr;
        }
        
        return true;
    }
  
  private:
    unsigned int m_init_size;//初始大小
    unsigned int m_grow_size;//每次增长大小
  private://内存块链表
    st_memblock *m_head_ptr;
    st_memblock *m_tail_ptr;
#ifdef SINGLE_UNITTEST //单元测试
  public:
    st_memblock* get_head_ptr() { return m_head_ptr; }
    st_memblock* get_tail_ptr() { return m_tail_ptr; }
    unsigned int get_init_size(){ return m_init_size;}
#endif
};
#endif
