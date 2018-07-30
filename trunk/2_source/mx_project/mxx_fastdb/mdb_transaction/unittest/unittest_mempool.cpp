#include <stdio.h>
#include <string.h>

#include "gtest/gtest.h"

#include "mdb_transac_mem.h"

TEST(CTransacMempool,use)
{
    CTransacMempool mempool(0);
    
    void *ptr=NULL;
    
    //初始内存块链表为空
    EXPECT_TRUE(NULL==mempool.get_head_ptr());
    EXPECT_TRUE(NULL==mempool.get_tail_ptr());

    st_memblock *head_ptr=NULL;
    st_memblock *tail_ptr=NULL;
    st_memblock *memblock_ptr1=NULL;
    st_memblock *memblock_ptr2=NULL;
    st_memblock *memblock_ptr3=NULL;
    st_memblock *memblock_ptr4=NULL;
    
    //产生新内存块: 可用空间大小12.......................
    printf("mempool::alloc(12),create new memblock 1...\n");
    ptr=mempool.alloc(12);
    EXPECT_TRUE(ptr!=NULL);
    memblock_ptr1=mempool.get_tail_ptr();
    EXPECT_TRUE(NULL!=memblock_ptr1);
    EXPECT_TRUE(memblock_ptr1==mempool.get_tail_ptr());
    EXPECT_EQ(memblock_ptr1->buff_size, 12);
    EXPECT_EQ(memblock_ptr1->used_size, 12);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr1);
    EXPECT_TRUE(memblock_ptr1->next==NULL);
    //EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    //EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    //EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    
    //产生新内存块; 可用空间大小=2*上一个可用空间=24.................
    printf("mempool::alloc(24),create new memblock 2...\n");
    ptr=mempool.alloc(24);
    EXPECT_TRUE(ptr!=NULL);
    memblock_ptr2=mempool.get_tail_ptr();
    EXPECT_TRUE(NULL!=memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2==mempool.get_tail_ptr());
    EXPECT_EQ(memblock_ptr2->buff_size, 24);
    EXPECT_EQ(memblock_ptr2->used_size, 24);
    
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==NULL);
    //EXPECT_TRUE(memblock_ptr3->next==NULL);
    //EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    
    //产生新内存块; 可用空间大小=2*上一个可用空间=48..........
    printf("mempool::alloc(24),create new memblock 3 size=(48)...\n");
    ptr=mempool.alloc(24);
    EXPECT_TRUE(ptr!=NULL);
    memblock_ptr3=mempool.get_tail_ptr();
    EXPECT_TRUE(NULL!=memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3==mempool.get_tail_ptr());
    EXPECT_EQ(memblock_ptr3->buff_size, 48);
    EXPECT_EQ(memblock_ptr3->used_size, 24);
    
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==NULL);
    //EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    //没有产生新内存块;......................
    printf("mempool::alloc(24),no new memblock...\n");
    ptr=mempool.alloc(24);
    EXPECT_TRUE(ptr!=NULL);
    //memblock_ptr3=mempool.get_tail_ptr();
    EXPECT_TRUE(NULL!=memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3==mempool.get_tail_ptr());
    EXPECT_EQ(memblock_ptr3->buff_size, 48);
    EXPECT_EQ(memblock_ptr3->used_size, 48);
 
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==NULL);
    //EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    //产生新内存块; 可用空间大小=2*上一个可用空间=96...............
    printf("mempool::alloc(12),create new memblock 3 size=(96)...\n");
    ptr=mempool.alloc(12);
    EXPECT_TRUE(ptr!=NULL);
    memblock_ptr4=mempool.get_tail_ptr();
    EXPECT_TRUE(NULL!=memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4==mempool.get_tail_ptr());
    EXPECT_EQ(memblock_ptr4->buff_size, 96);
    EXPECT_EQ(memblock_ptr4->used_size, 12);
 
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    //释放所有空间,但内存池不释放内存
    printf("mempool::free...\n");
    mempool.free();
    
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr1);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    EXPECT_EQ(memblock_ptr1->buff_size, 12);
    EXPECT_EQ(memblock_ptr1->used_size, 0);
    EXPECT_EQ(memblock_ptr2->buff_size, 24);
    EXPECT_EQ(memblock_ptr2->used_size, 0);
    EXPECT_EQ(memblock_ptr3->buff_size, 48);
    EXPECT_EQ(memblock_ptr3->used_size, 0);
    EXPECT_EQ(memblock_ptr4->buff_size, 96);
    EXPECT_EQ(memblock_ptr4->used_size, 0);
    
    printf("\n\n");
    int alloc_size=4;
    //重复使用内存池
    st_memblock * tmp_pool_ptr=NULL;
    printf("reuse mempool...\n");
    printf("1. alloc(4) on block1..\n");
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr1);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));
    EXPECT_EQ(tmp_pool_ptr->buff_size, 12);
    EXPECT_EQ(tmp_pool_ptr->used_size, 4);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr1);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    alloc_size=3;
    printf("2. alloc(%d) on block1..\n", alloc_size);
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr1);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));//检查分配指针的位置
    EXPECT_EQ(tmp_pool_ptr->buff_size, 12);
    EXPECT_EQ(tmp_pool_ptr->used_size, 7);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr1);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    
    alloc_size=13;
    printf("3. alloc(%d) on block 2..\n", alloc_size);
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr2);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));//检查分配指针的位置
    EXPECT_EQ(tmp_pool_ptr->buff_size, 24);
    EXPECT_EQ(tmp_pool_ptr->used_size, 13);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    
    alloc_size=10;
    printf("4. alloc(%d) on block 2..\n", alloc_size);
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr2);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));//检查分配指针的位置
    EXPECT_EQ(tmp_pool_ptr->buff_size, 24);
    EXPECT_EQ(tmp_pool_ptr->used_size, 23);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    alloc_size=25;
    printf("5. alloc(%d) on block 3..\n", alloc_size);
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr3);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));//检查分配指针的位置
    EXPECT_EQ(tmp_pool_ptr->buff_size, 48);
    EXPECT_EQ(tmp_pool_ptr->used_size, 25);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==NULL);
    
    
    alloc_size=128;
    st_memblock *memblock_ptr5=NULL;
    printf("6. alloc(%d) on block 5 (>all block size, need to create new block)..\n", alloc_size);
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =memblock_ptr5=mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr5);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));//检查分配指针的位置
    EXPECT_EQ(tmp_pool_ptr->buff_size, 192);
    EXPECT_EQ(tmp_pool_ptr->used_size, 128);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr5);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==memblock_ptr5);
    EXPECT_TRUE(memblock_ptr5->next==NULL);
    
    alloc_size=30;
    printf("7. alloc(%d) on block 5 ..\n", alloc_size);
    ptr=mempool.alloc(alloc_size);
    tmp_pool_ptr =mempool.get_tail_ptr();
    EXPECT_TRUE(ptr!=NULL);
    EXPECT_TRUE(tmp_pool_ptr==memblock_ptr5);
    EXPECT_TRUE(ptr==tmp_pool_ptr->buff_ptr+(tmp_pool_ptr->used_size-alloc_size));//检查分配指针的位置
    EXPECT_EQ(tmp_pool_ptr->buff_size, 192);
    EXPECT_EQ(tmp_pool_ptr->used_size, 158);
    //检查内存块链表
    head_ptr = mempool.get_head_ptr();
    tail_ptr = mempool.get_tail_ptr();
    EXPECT_TRUE(head_ptr==memblock_ptr1);
    EXPECT_TRUE(tail_ptr==memblock_ptr5);
    EXPECT_TRUE(memblock_ptr1->next==memblock_ptr2);
    EXPECT_TRUE(memblock_ptr2->next==memblock_ptr3);
    EXPECT_TRUE(memblock_ptr3->next==memblock_ptr4);
    EXPECT_TRUE(memblock_ptr4->next==memblock_ptr5);
    EXPECT_TRUE(memblock_ptr5->next==NULL);
    
}
int main(int argc, char **argv)
{
    printf("sizeof(st_memblock)=[%ld]\n", sizeof(st_memblock));
    
    testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
    
    return 0;
}
