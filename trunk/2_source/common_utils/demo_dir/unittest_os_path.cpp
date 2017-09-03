#include "os_path.h"

#include "gtest/gtest.h"

#include <stdio.h>
//测试案例: 判断路径指定的文件是否存在
TEST(os_path, file_exists)
{
   char path[256]={0};
   strcpy(path, "./test_file_exist.txt");
   
   EXPECT_TRUE(!os_path_exists(path));//文件不存在

   FILE *fp=fopen(path, "a+");
   fputs("this is for file_exists unit test! should be deleted after unit test!", fp);
   fclose(fp);

   EXPECT_TRUE(os_path_exists(path));//文件存在

   remove(path);
}

//测试案例: 获取文件大小
TEST(os_path, file_getsize)
{
   char path[256]={0};
   strcpy(path, "./test_file_size.txt");

   //EXPECT_TRUE(!os_path_exists(path));//文件不存在

   FILE *fp=fopen(path, "a+");
   fputs("this is unit test for calc file size! should be deleted after unit test!", fp);
   fclose(fp);
   
   int filesize=os_path_file_getsize(path);
   EXPECT_GT(filesize, 0);
   printf("filesize=[%d]\n", filesize);

   remove(path);
}


//测试案例: 获取路径中的文件名(也可能是目录名)
TEST(os_path, get_base_name)
{
   char path[256]={0};//路径
   char basename[64]={0};//期望结果
   char real_basename[64]={0};//实际执行结果
   int rc;

   //-------------------------------------------------------------
   //正常情况
   strcpy(basename, "test_base_name");
   sprintf(path, "/root/home/%s", basename);
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   //正常情况: basename后含分隔符
   strcpy(basename, "test_base_name");
   sprintf(path, "/root/home/%s/", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);
   
   //正常情况: basename前多个连续分隔符
   strcpy(basename, "test_base_name");
   sprintf(path, "/root/home///%s", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   //正常情况: basename后多个连续分隔符
   strcpy(basename, "test_base_na0me");
   sprintf(path, "/root/home/%s///", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);
  
   //正常情况: path不含任何分隔符
   strcpy(basename, "test_base_name");
   sprintf(path, "%s", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   //--------------------------------------------------------------
   //边界情况(basename长度为1)
   strcpy(basename, "t");
   sprintf(path, "/root/home/%s", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   strcpy(basename, "t");
   sprintf(path, "/root//home/%s/", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   strcpy(basename, "t");
   sprintf(path, "/root/home/%s/", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   strcpy(basename, "t");
   sprintf(path, "/root/home/%s///", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   strcpy(basename, "t");
   sprintf(path, "%s", basename);
   *real_basename='\0';
   rc=os_path_basename(path, real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(basename, real_basename);

   //-----------------------------------------------------------------
   //异常情况: path=NULL
   //strcpy(basename, "t");
   //sprintf(path, "%s", basename);
   *real_basename='\0';
   rc=os_path_basename(NULL, real_basename, sizeof(real_basename));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_basename[0]);

   //异常情况: path=空字符串
   path[0]='\0';
   *real_basename='\0';
   rc=os_path_basename(NULL, real_basename, sizeof(real_basename));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_basename[0]);

   //异常情况: 字符串常量: 空字符串
   *real_basename='\0';
   rc=os_path_basename("", real_basename, sizeof(real_basename));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_basename[0]);

   //异常情况: 字符常量
   *real_basename='\0';
   rc=os_path_basename("/root/home/test_base_name", real_basename, sizeof(real_basename));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ("test_base_name", real_basename);

   //异常情况: 输出缓存不足
   strcpy(basename, "tst_hello_this_is_basename");
   sprintf(path, "/root/home/%s", basename);
   *real_basename='\0';
   int buff_size=strlen(basename)-4;
   rc=os_path_basename(path, real_basename, buff_size);
   EXPECT_EQ(rc,0);
   EXPECT_STRNE(basename, real_basename);
   rc=strncmp(basename, real_basename, buff_size);
   EXPECT_NE(rc,0);
   rc=strncmp(basename, real_basename, buff_size-1);
   EXPECT_EQ(rc,0);

}


//测试案例: 获取路径中的父目录
TEST(os_path, get_dirname)
{
   char path[256]={0};
   char dirname[128]={0};
   char real_dirname[128]={0};//找到的父目录
   int rc=0;

   //正常情况:
   strcpy(dirname, "/root/home/git");
   sprintf(path, "%s/filename", dirname);
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(dirname, real_dirname);

   strcpy(dirname, "/root/home/git");
   sprintf(path, "%s///filename", dirname);
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(dirname, real_dirname);

   strcpy(dirname, "/root/home/git");
   sprintf(path, "%s///filename///", dirname);
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(dirname, real_dirname);

   strcpy(dirname, "/");
   sprintf(path, "/root");
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(dirname, real_dirname);

   strcpy(dirname, "/");
   sprintf(path, "///root");
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(dirname, real_dirname);

   strcpy(dirname, "/");
   sprintf(path, "////root///");
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ(dirname, real_dirname);

   strcpy(dirname, "/");
   sprintf(path, "////root///");
   real_dirname[0]='\0';
   rc=os_path_dirname("/root/home/git//", real_dirname, sizeof(real_dirname));
   EXPECT_EQ(rc,0);
   EXPECT_STREQ("/root/home", real_dirname);

   //异常情况
   strcpy(dirname, "");
   sprintf(path, "root");
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_dirname[0]);

   strcpy(dirname, "");
   sprintf(path, "root/");
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_dirname[0]);

   strcpy(dirname, "");
   sprintf(path, "root//%s", dirname);
   real_dirname[0]='\0';
   rc=os_path_dirname(path, real_dirname, sizeof(real_dirname));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_dirname[0]);

   strcpy(dirname, "");
   sprintf(path, "root%s", dirname);
   real_dirname[0]='\0';
   rc=os_path_dirname(NULL, real_dirname, sizeof(real_dirname));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_dirname[0]);

   strcpy(dirname, "");
   sprintf(path, "root%s", dirname);
   real_dirname[0]='\0';
   rc=os_path_dirname("", real_dirname, sizeof(real_dirname));
   EXPECT_LT(rc,0);
   EXPECT_EQ('\0', real_dirname[0]);

   //异常情况: 缓存长度不足
   strcpy(dirname, "/root/home/git");
   sprintf(path, "%s///filename///", dirname);
   real_dirname[0]='\0';
   int buf_size=strlen(dirname)-4;
   rc=os_path_dirname(path, real_dirname, buf_size);
   EXPECT_EQ(rc,0);
   EXPECT_STRNE(dirname, real_dirname);
   rc=strncmp(dirname, real_dirname, buf_size);
   EXPECT_NE(rc,0);
   rc=strncmp(dirname, real_dirname, buf_size-1);
   EXPECT_EQ(rc,0);

   strcpy(dirname, "/root/home/git");
   sprintf(path, "%s///filename///", dirname);
   real_dirname[0]='\0';
   buf_size=strlen(dirname);
   rc=os_path_dirname(path, real_dirname, buf_size);
   EXPECT_EQ(rc,0);
   EXPECT_STRNE(dirname, real_dirname);
   rc=strncmp(dirname, real_dirname, buf_size);
   EXPECT_NE(rc,0);
   rc=strncmp(dirname, real_dirname, buf_size-1);
   EXPECT_EQ(rc,0);
}

//测试案例: 递归创建目录
TEST(os_path, mkdir)
{
    char path[256]={0};
    int rc=0;

    strcpy(path, "/tmp/sub_path1/sub_path2/sub_path3/sub_path4");
    rc=os_path_mkdir(path);
    EXPECT_EQ(rc,0);
    rc=access(path, 0);
    EXPECT_EQ(rc,0);

    strcpy(path, "/tmp/sub_path1/sub_path2/sub_path3/sub_path4/sub_path5///");
    rc=os_path_mkdir(path);
    EXPECT_EQ(rc,0);
    rc=access(path, 0);
    EXPECT_EQ(rc,0);
}

//测试案例: 判断路径是目录
TEST(os_path, isdir)
{
     char path[256]={0};
     char filename[64]={0};

     int rc=0;
     strcpy(path, "./dir_path");
     rc=mkdir(path, 0755);
     EXPECT_EQ(rc,0);

     strcpy(filename, "./file_test");
     FILE *fp=fopen(filename, "a+");
     fclose(fp);
     
     rc=os_path_isdir(path);
     EXPECT_EQ(rc,1);

     rc=os_path_isdir(filename);
     EXPECT_EQ(rc,0);

     rc=os_path_isfile(path);
     EXPECT_EQ(rc,0);
  
     rc=os_path_isfile(filename);
     EXPECT_EQ(rc, 1);

     remove(filename);
     rmdir(path);
}

//测试案例: 判断路径是否是绝对路径
TEST(os_path, isabs)
{
    char path[256]={0};

    int rc=0;

    strcpy(path, "/home/root");
    rc=os_path_isabs(path);
    EXPECT_EQ(rc, 1);

    strcpy(path, "/");
    rc=os_path_isabs(path);
    EXPECT_EQ(rc, 1);

    strcpy(path, "home/root");
    rc=os_path_isabs(path);
    EXPECT_EQ(rc, 0);

    strcpy(path, "c:/");
    rc=os_path_isabs(path);
    EXPECT_EQ(rc, 1);

    strcpy(path, "./");
    rc=os_path_isabs(path);
    EXPECT_EQ(rc, 0);
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.
    return RUN_ALL_TESTS();
}
