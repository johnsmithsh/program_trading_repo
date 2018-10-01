#include "mxx_mem.h"

#include "gtest/gtest.h"

#include <unistd.h>
#include <stdio.h>

#include <errno.h>
#include <string.h>


TEST(mxx_varmem, create_destroy)
{
   mxx_varmem_t *mem_ptr=NULL;
   
   mem_ptr=mxx_varmem_create(0);
   ASSERT_EQ(NULL, mem_ptr);
   
   size_t buffsize=32;
   mem_ptr=mxx_varmem_create(buffsize);
   ASSERT_TRUE(NULL!=mem_ptr);
   ASSERT_EQ(buffsize, mem_ptr->size);
   ASSERT_EQ(0,        mem_ptr->membuff.length);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[1]);
   
   mxx_varmem_destroy(mem_ptr);
   ASSERT_EQ(NULL,     mem_ptr);
}

TEST(mxx_varmem, mem_to_varmem)
{
   unsigned char buff[64]={0};
   
   mxx_varmem_t *mem_ptr=mxx_mem_to_varmem(buff, sizeof(buff));

   ASSERT_TRUE(NULL != mem_ptr);
   ASSERT_EQ((unsigned char *)buff,             (unsigned char *)mem_ptr);
   ASSERT_EQ(sizeof(buff)-sizeof(mxx_varmem_t), mem_ptr->size);
   ASSERT_EQ(buff+sizeof(mxx_varmem_t),         mem_ptr->membuff.databuff);
}

TEST(mxx_varmem, assign)
{  
   int rc=0;
   mxx_varmem_t *mem_ptr=mxx_varmem_create(64);

   ASSERT_TRUE(NULL!=mem_ptr);
   ASSERT_TRUE(64==mem_ptr->size);
   
   char buff[64]="hello";
   rc=mxx_varmem_assign(mem_ptr, NULL, strlen(buff));
   EXPECT_EQ(64,  mem_ptr->size);
   EXPECT_EQ(0,        rc);
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   EXPECT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, 0);
   EXPECT_EQ(64,  mem_ptr->size);
   EXPECT_EQ(0,        rc);
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   EXPECT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, strlen(buff));
   EXPECT_EQ(64,  mem_ptr->size);
   EXPECT_EQ(0,        rc);
   EXPECT_EQ(strlen(buff),        mem_ptr->membuff.length);
   EXPECT_TRUE(0==strcmp(buff,     (char *)mem_ptr->membuff.databuff));
   
   int len=0;
   strcpy(buff, "world");
   len=3;
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, len);
   EXPECT_EQ(sizeof(buff),  mem_ptr->size);
   EXPECT_EQ(0,        rc);
   EXPECT_EQ(len,        mem_ptr->membuff.length);
   EXPECT_TRUE(0==strncmp(buff,     (char *)mem_ptr->membuff.databuff, len));
   
   strcpy(buff, "howareyou");
   len=7;
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, len);
   EXPECT_EQ(sizeof(buff),  mem_ptr->size);
   EXPECT_EQ(0,        rc);
   EXPECT_EQ(len,        mem_ptr->membuff.length);
   EXPECT_TRUE(0==strncmp(buff,     (char *)mem_ptr->membuff.databuff, len));
   
   char buff2[128]={0};
   memset(buff2, 'a', sizeof(buff2)-1);
   len = sizeof(buff2);
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff2, len);
   EXPECT_EQ(-1,  rc);
   
   mxx_varmem_destroy(mem_ptr);
}

TEST(mxx_varmem, clear)
{  
   int rc=0;
   size_t buffsize=64;
   mxx_varmem_t *mem_ptr=mxx_varmem_create(64);

   ASSERT_TRUE(NULL!=mem_ptr);
   
   int len = 0;
   char buff[64]="hello";
   len = strlen(buff);
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, len);
   EXPECT_EQ(buffsize,  mem_ptr->size);
   EXPECT_EQ(0,         rc);
   EXPECT_EQ(len,       mem_ptr->membuff.length);
   EXPECT_TRUE(0==strncmp(buff,     (char *)mem_ptr->membuff.databuff, len));
   
   mxx_varmem_clear(mem_ptr);
   EXPECT_EQ(buffsize, mem_ptr->size);
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[1]);
   
   mxx_varmem_destroy(mem_ptr);
}

TEST(mxx_varmem, address_and_freeaddress_and_buffsize_availsize_datalen)
{
   int rc=0;
   size_t buffsize=64;
   mxx_varmem_t *mem_ptr=mxx_varmem_create(buffsize);
   
   ASSERT_TRUE(NULL!=mem_ptr);
   
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[1]);
   
   unsigned char *data_ptr = mxx_varmem_address(mem_ptr);
   unsigned char *free_ptr = mxx_varmem_freeaddress(mem_ptr);
   EXPECT_TRUE(data_ptr == mem_ptr->membuff.databuff);
   EXPECT_TRUE(free_ptr == mem_ptr->membuff.databuff);
   EXPECT_TRUE(0        == mem_ptr->membuff.length);
   EXPECT_TRUE(0        == *free_ptr);
   size_t capacity_size = mxx_varmem_buffsize(mem_ptr);
   size_t avail_size    = mxx_varmem_availsize(mem_ptr);
   size_t data_size     = mxx_varmem_datalen(mem_ptr);
   EXPECT_EQ(capacity_size,   mem_ptr->size);
   EXPECT_EQ(avail_size,      buffsize);
   EXPECT_EQ(data_size,       0);
   
   int len = 0;
   char buff[64]="hello";
   len = strlen(buff);
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, len);
   data_ptr = mxx_varmem_address(mem_ptr);
   free_ptr = mxx_varmem_freeaddress(mem_ptr);
   EXPECT_TRUE(data_ptr == mem_ptr->membuff.databuff);
   EXPECT_TRUE(free_ptr == mem_ptr->membuff.databuff+len);
   EXPECT_TRUE(len      == mem_ptr->membuff.length);
   EXPECT_TRUE(0        == *free_ptr);
   EXPECT_STREQ((char *)data_ptr, (char *)buff);
   capacity_size = mxx_varmem_buffsize(mem_ptr);
   avail_size    = mxx_varmem_availsize(mem_ptr);
   data_size     = mxx_varmem_datalen(mem_ptr);
   EXPECT_EQ(buffsize,   mem_ptr->size);
   EXPECT_EQ(avail_size, buffsize-len);
   EXPECT_EQ(data_size,       len);
   
   
   strcpy(buff,"hello world");
   len = strlen(buff);
   rc=mxx_varmem_assign(mem_ptr, (unsigned char *)buff, len);
   data_ptr = mxx_varmem_address(mem_ptr);
   free_ptr = mxx_varmem_freeaddress(mem_ptr);
   EXPECT_TRUE(data_ptr == mem_ptr->membuff.databuff);
   EXPECT_TRUE(free_ptr == mem_ptr->membuff.databuff+len);
   EXPECT_TRUE(len      == mem_ptr->membuff.length);
   EXPECT_TRUE(0        == *free_ptr);
   EXPECT_STREQ((char *)data_ptr, (char *)buff);
   capacity_size = mxx_varmem_buffsize(mem_ptr);
   avail_size    = mxx_varmem_availsize(mem_ptr);
   data_size     = mxx_varmem_datalen(mem_ptr);
   EXPECT_EQ(buffsize,   mem_ptr->size);
   EXPECT_EQ(avail_size, buffsize-len);
   EXPECT_EQ(data_size,       len);
   
   mxx_varmem_destroy(mem_ptr);
}

void commn_test(mxx_varmem_t *mem_ptr, size_t buffsize)
{
   //mxx_varmem_t *mem_ptr=mxx_varmem_create(buffsize);
   
   ASSERT_TRUE(NULL!=mem_ptr);
   
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[1]);
   
   unsigned char *data_ptr = mxx_varmem_address(mem_ptr);
   unsigned char *free_ptr = mxx_varmem_freeaddress(mem_ptr);
   EXPECT_TRUE(data_ptr == mem_ptr->membuff.databuff);
   EXPECT_TRUE(free_ptr == mem_ptr->membuff.databuff);
   EXPECT_TRUE(0        == mem_ptr->membuff.length);
   EXPECT_TRUE(0        == *free_ptr);
   size_t capacity_size = mxx_varmem_buffsize(mem_ptr);
   size_t avail_size    = mxx_varmem_availsize(mem_ptr);
   size_t data_size     = mxx_varmem_datalen(mem_ptr);
   EXPECT_EQ(capacity_size,   mem_ptr->size);
   EXPECT_EQ(avail_size,      buffsize);
   EXPECT_EQ(data_size,       0);
   return;
}

void data_test(mxx_varmem_t *mem_ptr, size_t buffsize, unsigned char *expect_data_ptr, size_t expect_data_len)
{
   unsigned char *data_ptr = mxx_varmem_address(mem_ptr);
   unsigned char *free_ptr = mxx_varmem_freeaddress(mem_ptr);
   EXPECT_TRUE(data_ptr == mem_ptr->membuff.databuff);
   if(buffsize > expect_data_len)
   {
      EXPECT_TRUE(free_ptr == mem_ptr->membuff.databuff+expect_data_len);
   }
   else
   {
       EXPECT_TRUE(free_ptr==NULL);
   }
   EXPECT_TRUE(expect_data_len      == mem_ptr->membuff.length);
   if(buffsize > expect_data_len)
   {
       EXPECT_TRUE(0        == *free_ptr);
   }
   EXPECT_TRUE(0        == memcmp(data_ptr, expect_data_ptr,expect_data_len));
   
   size_t capacity_size = mxx_varmem_buffsize(mem_ptr);
   size_t avail_size    = mxx_varmem_availsize(mem_ptr);
   size_t data_size     = mxx_varmem_datalen(mem_ptr);
   EXPECT_EQ(buffsize,   mem_ptr->size);
   EXPECT_EQ(avail_size, buffsize-expect_data_len);
   EXPECT_EQ(data_size,       expect_data_len);
}

TEST(mxx_varmem, append)
{
   int rc=0;
   size_t buffsize=64;
   mxx_varmem_t *mem_ptr=mxx_varmem_create(buffsize);
   
   ASSERT_TRUE(NULL!=mem_ptr);
   
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[1]);
   
   int len = 0;
   char buff[64];
   //追加指针为NULL
   rc=mxx_varmem_append(mem_ptr, NULL, 10);
   EXPECT_EQ(0, rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, 0);
   //追加长度为0
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)buff, 0);
   EXPECT_EQ(0, rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, 0);
   
   //追加字符超过缓冲区
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)buff, 65);
   EXPECT_TRUE(0>rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, 0);
   

   //第一次追加:正常追加
   strcpy(buff, "hello");
   len = strlen(buff);
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)buff, len);
   EXPECT_EQ(0,rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //第二次追加: 正常追加
   char append_buff[128];
   int append_len=0;
   strcpy(append_buff, ",world");
   append_len=strlen(append_buff);
   strcat(buff, append_buff);
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)append_buff, append_len);
   len=strlen(buff);
   EXPECT_EQ(0, rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //第三次追加: 正常追加
   memset(append_buff, 0, sizeof(append_buff));
   strcpy(append_buff, "! I am OK.");
   append_len=strlen(append_buff);
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)append_buff, append_len);
   EXPECT_EQ(0, rc);
   
   strcat(buff, append_buff);
   len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //第四次追加: 要溢出了
   memset(append_buff, 0, sizeof(append_buff));
   for(int i=0; i<buffsize; ++i)
       append_buff[i]='a';
   append_len=strlen(append_buff);
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)append_buff, append_len);
   EXPECT_TRUE(0>rc);
   //长度未变,数据未变
   //strcat(buff, append_buff);
   //len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //第5次追加: 追加长度刚好等于剩余空闲长度
   memset(append_buff, 0, sizeof(append_buff));
   for(int i=0; i<buffsize-strlen(buff); ++i)
       append_buff[i]='b';
   append_len=strlen(append_buff);
   rc=mxx_varmem_append(mem_ptr, (unsigned char *)append_buff, append_len);
   EXPECT_EQ(0, rc);
   //长度未变,数据未变
   strcat(buff, append_buff);
   len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   mxx_varmem_destroy(mem_ptr);
}

TEST(mxx_varmem, insert)
{
   int rc=0;
   size_t buffsize=64;
   mxx_varmem_t *mem_ptr=mxx_varmem_create(buffsize);
   
   ASSERT_TRUE(NULL!=mem_ptr);
   
   EXPECT_EQ(0,        mem_ptr->membuff.length);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[0]);
   ASSERT_EQ('\0',     mem_ptr->membuff.databuff[1]);
   
   int len = 0;
   char buff[64]={0};
   //插入指针为NULL
   rc=mxx_varmem_insert(mem_ptr, 0, NULL, 10);
   EXPECT_EQ(0, rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, 0);
   //插入长度为0
   rc=mxx_varmem_insert(mem_ptr, 0, (unsigned char *)buff, 0);
   EXPECT_EQ(0, rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, 0);
   
   //插入字符超过缓冲区
   rc=mxx_varmem_insert(mem_ptr, 0, (unsigned char *)buff, 65);
   EXPECT_TRUE(0>rc);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, 0);
   
   char insert_buff[128]={0};
   int insert_len=0;
   
   //1. 正常插入: 缓存区没有数据
   memset(insert_buff, 0, sizeof(insert_buff));
   strcpy(insert_buff, "hello");
   insert_len=strlen(insert_buff);
   rc=mxx_varmem_insert(mem_ptr, 0, (unsigned char *)insert_buff, insert_len);
   EXPECT_EQ(0, rc);
   memset(buff, 0, sizeof(buff));
   strcpy(buff, "hello");
   len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //2.正常插入: 从尾部插入
   memset(insert_buff, 0, sizeof(insert_buff));
   strcpy(insert_buff, ",world!");
   insert_len = strlen(insert_buff);
   rc=mxx_varmem_insert(mem_ptr, strlen(buff), (unsigned char *)insert_buff, insert_len);
   EXPECT_EQ(0, rc);
   memset(buff, 0, sizeof(buff));
   strcpy(buff, "hello,world!");
   len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //3.正常插入: 从头部插入
   memset(insert_buff, 0, sizeof(insert_buff));
   strcpy(insert_buff, "how are you.");
   insert_len = strlen(insert_buff);
   rc=mxx_varmem_insert(mem_ptr, 0, (unsigned char *)insert_buff, insert_len);
   EXPECT_EQ(0, rc);
   memset(buff, 0, sizeof(buff));
   strcpy(buff, "how are you.hello,world!");
   len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //4.正常插入: 从中间插入
   memset(insert_buff, 0, sizeof(insert_buff));
   strcpy(insert_buff, "I am OK.");
   insert_len = strlen(insert_buff);
   rc=mxx_varmem_insert(mem_ptr, 12, (unsigned char *)insert_buff, insert_len);
   EXPECT_EQ(0, rc);
   memset(buff, 0, sizeof(buff));
   strcpy(buff, "how are you.I am OK.hello,world!");
   len=strlen(buff);
   data_test(mem_ptr, buffsize, (unsigned char *)buff, len);
   
   //5.正常插入: 从数据最后字符插入
   
   //6.正常插入: 从倒数第2个字符插入
   
   //8.正常插入: 从第2个字符插入
   
   //9.异常插入: 从最后字符之后插入,变为追加
   
   //10.异常插入: 插入长度超过缓存剩余长度
   
   mxx_varmem_destroy(mem_ptr);
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
}
