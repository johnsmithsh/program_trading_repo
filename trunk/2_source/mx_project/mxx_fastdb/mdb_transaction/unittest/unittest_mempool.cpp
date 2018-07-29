#include <stdio.h>
#include <string.h>

#include "gtest/gtest.h"

#include "mdb_transac_mem.h"

TEST(CTransacMempool,)
{
    CTransacMempool mempool(0);
    
    void *ptr=NULL;
    
    //产生新内存块: 可用空间大小12
    ptr=mempool.alloc(12);
    EXPECT_TRUE(ptr!=NULL);
    
    //产生新内存块; 可用空间大小=2*上一个可用空间=24
    ptr=mempool.alloc(24);
    EXPECT_TRUE(ptr!=NULL);
    
    //产生新内存块; 可用空间大小=2*上一个可用空间=48
    ptr=mempool.alloc(24);
    EXPECT_TRUE(ptr!=NULL);
    
    //没有产生新内存块; 
    ptr=mempool.alloc(24);
    EXPECT_TRUE(ptr!=NULL);
    
    //产生新内存块; 可用空间大小=2*上一个可用空间=48
    ptr=mempool.alloc(12);
    EXPECT_TRUE(ptr!=NULL);
    
    //释放所有空间,但内存池不释放内存
    mempool.free();
}
int main(int argc, char **argv)
{
    printf("sizeof(st_memblock)=[%ld]\n", sizeof(st_memblock));
    
    testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
    
    return 0;
}
