#include <stdio.h>

#include <errno.h>
#include <string.h>


#include "rtti_def.h"
#include "gtest/gtest.h"

int field_eq(ST_FIELD_DESC *ptr1, ST_FIELD_DESC *ptr2)
{
   printf("     index=[%d], name=[%s]\n", ptr1->field_index, ptr1->field_name);
   int rc= memcmp(ptr1, ptr2, sizeof(ST_FIELD_DESC));
   EXPECT_EQ(0, rc);

   EXPECT_EQ(ptr1->field_index, ptr2->field_index);
   if(ptr1->field_data_type != ptr2->field_data_type)
   	 EXPECT_EQ(1,0);
   EXPECT_STREQ(ptr1->field_name, ptr2->field_name);
   EXPECT_STREQ(ptr1->field_comment, ptr2->field_comment);
   EXPECT_EQ(ptr1->count, ptr2->count);
   EXPECT_EQ(ptr1->st_offset, ptr2->st_offset);
   EXPECT_EQ(ptr1->st_offset, ptr2->st_offset);
   EXPECT_EQ(ptr1->field_size,  ptr2->field_size);
   return 0;
}

struct st_sample
{
   char bs_flag;
   char sh_flag;
   char eo_flag;
   unsigned char if_success;
   short core;
   unsigned short umask;
   int entrust_qty;
   unsigned int done_qty;
   long serial_no;
   char cust_no[16];
};

//
TEST(RttiFieldDescriptor, RttiFieldDescriptor)
{

   CFieldDescriptor field_demo;

   ST_FIELD_DESC field_vec[]={
     {0, ST_FIELD_DESC::RDT_CHAR,  "bs_flag",     "买卖方向",    1, 0,  0, 1, NULL, NULL},
     {1, ST_FIELD_DESC::RDT_CHAR,  "sh_flag",    "投保标记",     1, 1,  1, 1, NULL, NULL},
     {2, ST_FIELD_DESC::RDT_CHAR,  "eo_flag",    "开平标记",     1, 2,  2, 1, NULL, NULL},
     {3, ST_FIELD_DESC::RDT_UCHAR, "if_success", "成功标记",     1, 3,  3, 1, NULL, NULL},
     {4, ST_FIELD_DESC::RDT_INT2,  "core_no",    "交易核心编码", 1, 4,  4, 2, NULL, NULL},
     {5, ST_FIELD_DESC::RDT_UINT2, "umask",      "委托掩码",     1, 6,  6, 2, NULL, NULL},
     {6, ST_FIELD_DESC::RDT_INT4,  "entrust_qty", "委托手数",    1, 8,  8, 4, NULL, NULL},
     {7, ST_FIELD_DESC::RDT_UINT4, "done_qty",   "成交手数",     1, 12, 12, 4, NULL, NULL},
     {8, ST_FIELD_DESC::RDT_LONG,  "serial_no",  "委托流水号",   1, 16, 16, 8, NULL, NULL},
     {9, ST_FIELD_DESC::RDT_STR,   "cust_no",    "客户号",       1, 24, 24, 16, NULL, NULL}
   };
   int rc;
   rc=field_demo.add_char("bs_flag", "买卖方向", 0);
   rc=field_demo.add_char("sh_flag", "投保标记", 1);
   rc=field_demo.add_char("eo_flag", "开平标记", 2);
   rc=field_demo.add_uchar("if_success", "成功标记", 3);
   rc=field_demo.add_int2("core_no", "交易核心编码", 4);
   rc=field_demo.add_uint2("umask", "委托掩码", 6);
   rc=field_demo.add_int4("entrust_qty", "委托手数", 8);
   rc=field_demo.add_uint4("done_qty", "成交手数", 12);
   rc=field_demo.add_long("serial_no", "委托流水号", 16);
   rc=field_demo.add_str("cust_no", "客户号", 24, 16);
   
   int count = field_demo.count();
   EXPECT_EQ(count, 10);

   int serial_size=field_demo.get_serial_size();
   int st_size = field_demo.get_st_size();
   EXPECT_EQ(serial_size, field_vec[9].st_offset   + field_vec[9].field_size);
   EXPECT_EQ(st_size,     field_vec[9].st_offset + field_vec[9].field_size);

   ST_FIELD_DESC *ptr_byindex,*ptr_byname,*dst_field_ptr;
   int index=0;
   char name[64];
   
   index=0;
   strcpy(name, "bs_flag");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=1;
   strcpy(name, "sh_flag");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=2;
   strcpy(name, "eo_flag");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=3;
   strcpy(name, "if_success");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=4;
   strcpy(name, "core_no");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=5;
   strcpy(name, "umask");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=6;
   strcpy(name, "entrust_qty");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=7;
   strcpy(name, "done_qty");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);


   index=8;
   strcpy(name, "serial_no");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   index=9;
   strcpy(name, "cust_no");
   ptr_byindex = field_demo.find_field_ptr(index);
   ptr_byname = field_demo.find_field_ptr(name);
   dst_field_ptr=&field_vec[index];
   printf("Test: index=[%d==%d], name=[%s=%s]\n", index, ptr_byname->field_index, name, ptr_byname->field_name);
   rc=field_eq(ptr_byindex, ptr_byname);
   rc=field_eq(ptr_byindex, dst_field_ptr);
   EXPECT_EQ(ptr_byindex->field_index, index);
   EXPECT_STREQ(ptr_byindex->field_name, name);

   //
   printf("\n\n测试案例: 重复添加问题...\n");
   ST_FIELD_DESC *ptr;
   rc=field_demo.add_uchar("bs_flag", "买卖标记2", 12);//第一个元素
   printf("[1st]重复添加bs_flag,uchar rc=[%d]...\n", rc);
   EXPECT_LT(rc,0);
   ptr=field_demo.find_field_ptr("bs_flag");
   dst_field_ptr=&field_vec[0];
   rc=field_eq(ptr, dst_field_ptr);//字段说明未发生变化

   rc=field_demo.add_uchar("entrust_qty", "委托手数2", 12);//中间元素
   printf("[midle]重复添加entrust_qty,uchar rc=[%d]...\n", rc);
   EXPECT_LT(rc,0);
   ptr=field_demo.find_field_ptr("entrust_qty");
   dst_field_ptr=&field_vec[6];
   rc=field_eq(ptr, dst_field_ptr);//字段说明未发生变化

   rc=field_demo.add_uchar("cust_no", "客户号2", 12);//最后一个元素
   printf("[end]重复添加[cust_no],uchar rc=[%d]...\n", rc);
   EXPECT_LT(rc,0);
   ptr=field_demo.find_field_ptr("cust_no");
   dst_field_ptr=&field_vec[9];
   rc=field_eq(ptr, dst_field_ptr);//字段信息未发生变化
 

   printf("\n\n测试案例: 引用修改...\n");
   index=3;
   ST_FIELD_DESC &field_ref = field_demo.find_field(index);
   char field_comment[64]="成功标记(mod by ref)";
   strcpy(field_ref.field_comment,field_comment);
   ptr=field_demo.find_field_ptr(index);
   EXPECT_STREQ(ptr->field_comment, field_comment);
   //恢复数据
   memset(ptr->field_comment, 0, sizeof(ptr->field_comment));
   strcpy(ptr->field_comment, field_vec[index].field_comment);//恢复数据

   printf("\n\n测试案例:引用失效\n");
   //index=23;
   //field_ref = field_demo.get_field(index);
   //printf("  失效引用, index=[%d], name=[%s]\n", field_ref.field_index, field_ref.field_name);

   printf("\n\n测试案例: 指针修改...\n");
   index=4;
   ST_FIELD_DESC *mod_ptr = field_demo.find_field_ptr(index);
   strcpy(field_comment,"交易核心编码(mod by pointer)");
   strcpy(mod_ptr->field_comment,field_comment);
   ptr=field_demo.find_field_ptr(index);
   EXPECT_STREQ(ptr->field_comment, field_comment);
   //恢复数据
   memset(ptr->field_comment, 0, sizeof(ptr->field_comment));
   strcpy(ptr->field_comment, field_vec[index].field_comment);//恢复数据
   
   //测试迭代器
   printf("\n\n测试案例: 迭代器...\n");
   CFieldDescriptor::iterator iter;
   iter=field_demo.begin();
   index=0;
   while(iter!=field_demo.end())
   {
      dst_field_ptr=&field_vec[index];
      printf("迭代器测试:index=[%d]\n", index);
      printf("   iter:   index=[%d], name=[%s],comment=[%s] \n", iter->field_index, iter->field_name, iter->field_comment);
      ST_FIELD_DESC des=*iter;
      rc=field_eq(&des, dst_field_ptr);//字段信息: 未发生变化

      ++index;
      ++iter;
   }

   //printf("\n\n一次跳过多个\n");
   //iter=field_demo.begin();
   //printf("  curr  index=[%d], name=[%s]\n", iter->field_index, iter->field_name);
   //iter = iter + 3;
   //printf("  next 3 index=[%d], name=[%s]\n", iter->field_index, iter->field_name);
   //EXPECT_EQ(iter->field_index, 3);
   

    
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
}
