#include <stdio.h>
#include <string.h>

#include "gtest/gtest.h"

#include "mdb_transac_mem.h"

//单元测试: 测试创建函数
TEST(memblock, create_delete)
{
    st_memblock *block_ptr=NULL;
    
    //
    int block_size=32;
    block_ptr = memblock_create(block_size);
    if(NULL==block_ptr)
    {
        printf("Error: memblock_create(%d)failed!\n", block_size);
        return;
    }
    
    EXPECT_EQ(block_ptr->buff_ptr, (void *)block_ptr + sizeof(st_memblock));
    EXPECT_EQ(block_ptr->buff_size, block_size - sizeof(st_memblock));
    EXPECT_EQ(block_ptr->used_size, 0);
    EXPECT_EQ(block_ptr->prev, (void *)NULL);
    EXPECT_EQ(block_ptr->next, (void *)NULL);
    
    st_memblock *block_ptr1=NULL;
    block_size = 100;
    block_ptr1 = memblock_create(block_size);
    if(NULL==block_ptr1)
    {
        printf("Error: memblock_create(%d)failed!\n", block_size);
        return;
    }
    EXPECT_EQ(block_ptr1->buff_ptr, (void *)block_ptr1 + sizeof(st_memblock));
    EXPECT_EQ(block_ptr1->buff_size, block_size - sizeof(st_memblock));
    EXPECT_EQ(block_ptr1->used_size, 0);
    EXPECT_EQ(block_ptr1->prev, (void *)NULL);
    EXPECT_EQ(block_ptr1->next, (void *)NULL);
    
    
    st_memblock *tmp_ptr=NULL;
    tmp_ptr=memblock_delete(block_ptr);
    EXPECT_EQ(tmp_ptr, (void *)NULL);
    
    tmp_ptr=memblock_delete(block_ptr1);
    EXPECT_EQ(tmp_ptr, (void *)NULL);
    
}

TEST(memblock,alloc_free)
{
    st_memblock *block_ptr=NULL;
    
    //
    int block_size=1024;
    block_ptr = memblock_create(block_size);
    if(NULL==block_ptr)
    {
        printf("Error: memblock_create(%d)failed!\n", block_size);
        return;
    }
    
    //申请内存1
    int alloc_size = 16;
    void *ptr1 = NULL;
    ptr1 = memblock_alloc(block_ptr, alloc_size);
    EXPECT_EQ(ptr1,                 block_ptr->buff_ptr);
    EXPECT_EQ(block_ptr->used_size,  16);
    EXPECT_EQ(block_ptr->buff_size, block_size-sizeof(st_memblock));//大小不会变化
    
    //申请内存2
    void *ptr2 = NULL;
    alloc_size=100;
    int use_size = block_ptr->used_size;
    ptr2 = memblock_alloc(block_ptr, alloc_size);
    EXPECT_EQ(ptr2,                  (void *)(block_ptr->buff_ptr+use_size));
    EXPECT_EQ(block_ptr->used_size,   use_size+alloc_size);
    EXPECT_EQ(block_ptr->buff_size, block_size-sizeof(st_memblock));//大小不会变化
    
    //申请内存3
    void *ptr3 = NULL;
    alloc_size=400;
    use_size = block_ptr->used_size;
    ptr3 = memblock_alloc(block_ptr, alloc_size);
    EXPECT_EQ(ptr3,                 (void *)(block_ptr->buff_ptr+use_size));
    EXPECT_EQ(block_ptr->used_size,  use_size+alloc_size);
    EXPECT_EQ(block_ptr->buff_size, block_size-sizeof(st_memblock));//大小不会变化

    //申请内存>可用内存,则无法申请
    use_size = block_ptr->used_size;
    void *ptr4=memblock_alloc(block_ptr, block_size);
    EXPECT_EQ(ptr4,                  (void *)NULL);//没有分配内存
    EXPECT_EQ(block_ptr->used_size,   use_size);//已使用长度未发生变化
    EXPECT_EQ(block_ptr->buff_size, block_size-sizeof(st_memblock));//大小不会变化
    
    //释放全部被占用空间
    memblock_free(block_ptr);
    EXPECT_EQ(block_ptr->used_size,   0);//已使用长度未发生变化
    EXPECT_EQ(block_ptr->buff_size, block_size-sizeof(st_memblock));//大小不会变化
    
    st_memblock *tmp_ptr=memblock_delete(block_ptr);//释放对象
    block_ptr = NULL;
}

TEST(memblock,insert_remove)
{
    int block_size = 128;
    st_memblock *block_ptr=NULL;
    st_memblock *tmp_block=NULL;
    
    st_memblock *block_ptr1= memblock_create(block_size);
    st_memblock *block_ptr2= memblock_create(block_size);
    st_memblock *block_ptr3= memblock_create(block_size);
    st_memblock *block_ptr4= memblock_create(block_size);
    st_memblock *block_ptr5= memblock_create(block_size);
    if((NULL==block_ptr1)||(NULL==block_ptr2)||(NULL==block_ptr3)||(NULL==block_ptr4)||(NULL==block_ptr5))
    {
        printf("Error: memblock_create(%d)failed!\n", block_size);
        return;
    }
    printf("insert 1: block_ptr4 memblock\n");
    tmp_block = mempool_insert(NULL, block_ptr4, true);
    block_ptr = tmp_block;
    EXPECT_EQ(tmp_block,   block_ptr4);
    EXPECT_EQ(tmp_block->prev,   (void *)NULL);
    EXPECT_EQ(tmp_block->next,   (void *)NULL);
    
    printf("insert 2: block_ptr1 memblock\n");
    tmp_block = mempool_insert(block_ptr4, block_ptr1, false);
    EXPECT_EQ(tmp_block,   block_ptr1);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr4);
    EXPECT_EQ(block_ptr4->prev,   block_ptr1);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    
    printf("insert 3: block_ptr3 memblock\n");
    tmp_block = mempool_insert(block_ptr4, block_ptr3, false);
    EXPECT_EQ(tmp_block,   block_ptr3);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr3);
    EXPECT_EQ(block_ptr3->prev,   block_ptr1);
    EXPECT_EQ(block_ptr3->next,   block_ptr4);
    EXPECT_EQ(block_ptr4->prev,   block_ptr3);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    
    printf("insert 4: block_ptr2 memblock\n");
    tmp_block = mempool_insert(block_ptr1, block_ptr2, true);
    EXPECT_EQ(tmp_block,   block_ptr2);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr2);
    EXPECT_EQ(block_ptr2->prev,   block_ptr1);
    EXPECT_EQ(block_ptr2->next,   block_ptr3);
    EXPECT_EQ(block_ptr3->prev,   block_ptr2);
    EXPECT_EQ(block_ptr3->next,   block_ptr4);
    EXPECT_EQ(block_ptr4->prev,   block_ptr3);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    
    printf("insert 5: block_ptr5 memblock\n");
    tmp_block = mempool_insert(block_ptr4, block_ptr5, true);
    EXPECT_EQ(tmp_block,   block_ptr5);
    //最终链表
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr2);
    EXPECT_EQ(block_ptr2->prev,   block_ptr1);
    EXPECT_EQ(block_ptr2->next,   block_ptr3);
    EXPECT_EQ(block_ptr3->prev,   block_ptr2);
    EXPECT_EQ(block_ptr3->next,   block_ptr4);
    EXPECT_EQ(block_ptr4->prev,   block_ptr3);
    EXPECT_EQ(block_ptr4->next,   block_ptr5);
    EXPECT_EQ(block_ptr5->prev,   block_ptr4);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    
    printf("remove: block_ptr3 memblock\n");
    tmp_block = mempool_remove(block_ptr3);
    EXPECT_EQ(tmp_block,   block_ptr4);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr2);
    EXPECT_EQ(block_ptr2->prev,   block_ptr1);
    EXPECT_EQ(block_ptr2->next,   block_ptr4);
    EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   block_ptr2);
    EXPECT_EQ(block_ptr4->next,   block_ptr5);
    EXPECT_EQ(block_ptr5->prev,   block_ptr4);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    printf("remove: block_ptr1 memblock\n");
    tmp_block = mempool_remove(block_ptr1);
    EXPECT_EQ(tmp_block,   block_ptr2);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr2->next,   block_ptr4);
    EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   block_ptr2);
    EXPECT_EQ(block_ptr4->next,   block_ptr5);
    EXPECT_EQ(block_ptr5->prev,   block_ptr4);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    printf("remove: block_ptr5 memblock\n");
    tmp_block = mempool_remove(block_ptr5);
    EXPECT_EQ(tmp_block,   (void *)NULL);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr2->next,   block_ptr4);
    EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   block_ptr2);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    EXPECT_EQ(block_ptr5->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    printf("remove: block_ptr2 memblock\n");
    tmp_block = mempool_remove(block_ptr2);
    EXPECT_EQ(tmp_block,   block_ptr4);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr2->next,   (void *)NULL);
    EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    EXPECT_EQ(block_ptr5->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    printf("remove: block_ptr4 memblock\n");
    tmp_block = mempool_remove(block_ptr4);
    EXPECT_EQ(tmp_block,   (void *)NULL);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr2->next,   (void *)NULL);
    EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    EXPECT_EQ(block_ptr5->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    printf("delete:  memblock\n");
    memblock_delete(block_ptr1);
    memblock_delete(block_ptr2);
    memblock_delete(block_ptr3);
    memblock_delete(block_ptr4);
    memblock_delete(block_ptr5);
    block_ptr1=NULL;
    block_ptr2=NULL;
    block_ptr3=NULL;
    block_ptr4=NULL;
    block_ptr5=NULL;
    
}

//单元测试: append向链表尾部添加数据块; delete后会从链表中移除被删除的内存块;
TEST(memblock,append_delete)
{   
    int block_size = 128;
    st_memblock *block_ptr=NULL;
    st_memblock *tmp_block=NULL;
    
    st_memblock *block_ptr1= memblock_create(block_size);
    st_memblock *block_ptr2= memblock_create(block_size);
    st_memblock *block_ptr3= memblock_create(block_size);
    st_memblock *block_ptr4= memblock_create(block_size);
    st_memblock *block_ptr5= memblock_create(block_size);
    if((NULL==block_ptr1)||(NULL==block_ptr2)||(NULL==block_ptr3)||(NULL==block_ptr4)||(NULL==block_ptr5))
    {
        printf("Error: memblock_create(%d)failed!\n", block_size);
        return;
    }
    
    mempool_append(NULL, block_ptr1);
    mempool_append(block_ptr1, block_ptr2);
    mempool_append(block_ptr2, block_ptr3);
    mempool_append(block_ptr2, block_ptr4);
    mempool_append(block_ptr2, block_ptr5);
    //最终链表
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr2);
    EXPECT_EQ(block_ptr2->prev,   block_ptr1);
    EXPECT_EQ(block_ptr2->next,   block_ptr3);
    EXPECT_EQ(block_ptr3->prev,   block_ptr2);
    EXPECT_EQ(block_ptr3->next,   block_ptr4);
    EXPECT_EQ(block_ptr4->prev,   block_ptr3);
    EXPECT_EQ(block_ptr4->next,   block_ptr5);
    EXPECT_EQ(block_ptr5->prev,   block_ptr4);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    tmp_block = memblock_delete(block_ptr3);
    EXPECT_EQ(tmp_block,   block_ptr4);
    EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr1->next,   block_ptr2);
    EXPECT_EQ(block_ptr2->prev,   block_ptr1);
    EXPECT_EQ(block_ptr2->next,   block_ptr4);
    //EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   block_ptr2);
    EXPECT_EQ(block_ptr4->next,   block_ptr5);
    EXPECT_EQ(block_ptr5->prev,   block_ptr4);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    tmp_block = memblock_delete(block_ptr1);
    EXPECT_EQ(tmp_block,   block_ptr2);
    //EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr2->next,   block_ptr4);
    //EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   block_ptr2);
    EXPECT_EQ(block_ptr4->next,   block_ptr5);
    EXPECT_EQ(block_ptr5->prev,   block_ptr4);
    EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    tmp_block = memblock_delete(block_ptr5);
    EXPECT_EQ(tmp_block,   (void *)NULL);
    //EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr2->next,   block_ptr4);
    //EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   block_ptr2);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr5->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    tmp_block = memblock_delete(block_ptr2);
    EXPECT_EQ(tmp_block,   block_ptr4);
    //EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr2->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    EXPECT_EQ(block_ptr4->prev,   (void *)NULL);
    EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr5->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    tmp_block = memblock_delete(block_ptr4);
    EXPECT_EQ(tmp_block,   (void *)NULL);
    //EXPECT_EQ(block_ptr1->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr1->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr2->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr2->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr3->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr4->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr4->next,   (void *)NULL);
    //EXPECT_EQ(block_ptr5->prev,   (void *)NULL);
    //EXPECT_EQ(block_ptr5->next,   (void *)NULL);
    
    block_ptr1=NULL;
    block_ptr2=NULL;
    block_ptr3=NULL;
    block_ptr4=NULL;
    block_ptr5=NULL;
}

//单元测试: 字节对齐
TEST(to_memalign,to_memalign)
{
    unsigned int mem_size;
    unsigned int align_size;
    
    unsigned int to_size;
    mem_size=0x00;
    to_size=to_memalign(mem_size, 1);
    EXPECT_EQ(to_size,   1);
    to_size=to_memalign(mem_size, 2);
    EXPECT_EQ(to_size,   2);
    to_size=to_memalign(mem_size, 4);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 8);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 18);
    EXPECT_EQ(to_size,   16);
    
    mem_size=0x01;
    to_size=to_memalign(mem_size, 1);
    EXPECT_EQ(to_size,   1);
    to_size=to_memalign(mem_size, 2);
    EXPECT_EQ(to_size,   2);
    to_size=to_memalign(mem_size, 4);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 8);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 16);
    EXPECT_EQ(to_size,   16);
    
    mem_size=0x02;
    to_size=to_memalign(mem_size, 1);
    EXPECT_EQ(to_size,   2);
    to_size=to_memalign(mem_size, 2);
    EXPECT_EQ(to_size,   2);
    to_size=to_memalign(mem_size, 4);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 8);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 16);
    EXPECT_EQ(to_size,   16);
    
    mem_size=0x03;
    to_size=to_memalign(mem_size, 1);
    EXPECT_EQ(to_size,   3);
    to_size=to_memalign(mem_size, 2);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 4);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 8);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 16);
    EXPECT_EQ(to_size,   16);
    
    
    mem_size=0x04;
    to_size=to_memalign(mem_size, 1);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 2);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 4);
    EXPECT_EQ(to_size,   4);
    to_size=to_memalign(mem_size, 8);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 16);
    EXPECT_EQ(to_size,   16);
    
    mem_size=0x08;
    to_size=to_memalign(mem_size, 1);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 2);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 4);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 8);
    EXPECT_EQ(to_size,   8);
    to_size=to_memalign(mem_size, 16);
    EXPECT_EQ(to_size,   16);
}

int main(int argc, char **argv)
{
    printf("sizeof(st_memblock)=[%ld]\n", sizeof(st_memblock));
    
    testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
}

