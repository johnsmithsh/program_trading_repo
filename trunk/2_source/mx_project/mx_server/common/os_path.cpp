
/*
 * 说明: 仿造python的os.path模块定义c语言常用的路径操作
 *
 * */
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h> //toupper/tolower

#include <limits.h>

#include <assert.h>
#include "os_path.h"

#ifdef _WIN32
#define OS_MKDIR(a) _mkdir(a)
#else
#define OS_MKDIR(a) mkdir((a),0755)
#endif


//判断字符是路径分隔符
//#define os_path_is_sep_char(ch) ( (PATH_SEP_CHAR_LINUX==(ch)) || (PATH_SEP_CHAR_WIN==(ch)) )

//功能: 获取path表示的绝对路径
//int os_path_abspath(char *path, char *abspath, int size)
//{
//    return -1;
//}

//功能: 获取该路径的父目录;
//参数:
//    [in]path: 路径信息
//    [out]dirname, size: 父目录缓存区和大小(含字符串后的'\0'); 注:size>=2;
//             size长度不足,则返回结果被截断;
//返回值: 0-成功; <0-失败;
//注:
//     1. 根目录的父目录是'/';
//     2. path中值包含文件名,不包含路径,则其父目录是".", 如path="basename"
//     3. path最后的分隔符不作为一级目录;
//     4. 说明: 
//          "/"        父目录是  "/";
//          "hello///" 父目录是 ""; 即没有找到父目录，返回值<0;
//          "hello"    父目录是 ""; 即没有找到父目录, 返回值<0;
int os_path_dirname(char *path, char *dirname, int size)
{
    assert(NULL!=path);
    assert(NULL!=dirname);
    
   *dirname='\0';
   if( (NULL==path) || ('\0'==*path) ) return -1;
   char *ptr=NULL;
   int len=0;

   //找最后一个不是分隔符的位置
   ptr=path+strlen(path)-1;
   while( (ptr>=path) && (os_path_is_sep_char(*ptr)) ) --ptr;
   if(ptr<path) //path中全是分隔符,如"/////"
   {
     strcpy(dirname, "/");
     return 0;
   }
  
   //找下一个分隔符位置
   while( (ptr>=path) && (!os_path_is_sep_char(*ptr)) )
   {
       ++len;
       --ptr;
   }
   if(ptr<path)//path不存在下一个分隔符(path格式:"home///"、"home"), 说明path本身就是文件名或目录名, 则父目录不存在,返回错误;
   {
     //*dirname='.';
     //*(dirname+1)='\0';
     *dirname='\0';
     return -2;
   }

   //拷贝父目录 
   len = ptr - path;
   if(len>=size)
      len = size-1;

   strncpy(dirname, path, len);
   *(dirname+len)='\0';
   
   //移除尾部的分隔符, 如"//root///"
   ptr=dirname+len-1;
   while((ptr>=dirname) && (os_path_is_sep_char(*ptr))) (*ptr--)='\0';
   if(ptr<dirname)//说明是根目录(path格式: "//////root"),根目录的服务目录还是根目录;
   {
      *dirname='/';
      *(dirname+1)='\0';
   }
   
   return 0;
}

//功能: 获取path的最后一个目录或文件
//参数:
//   [in]path: 路径
//   [out]basename, size: 返回的名字和缓存区长度;
//         size不足,则返回字符串被截断
//返回值: 0-成功; <0失败
int os_path_basename(const char *path, char *basename, int size)
{
   assert(NULL!=path);
   assert(NULL!=basename);
   
   *basename='\0';
   if( (NULL==path) || ('\0'==*path) ) return -1;
   const char *ptr=NULL;
   int len=0;
   //char ch='\0';
   
   //找到最后一个不是路径分隔符的位置;
   //  path可能是路径,最后一个字符是路径分隔符,如"/root/home///"
   ptr=path+strlen(path)-1;
   while( (ptr>=path) && (os_path_is_sep_char(*ptr)) ) --ptr;
   if(ptr<path) //说明path中全是分隔符, 故没有文件名信息;
     return -2;

   //找到下一个路径分隔符位置,统计记录长度
   while( (ptr>=path) && (!os_path_is_sep_char(*ptr)) )
   {
       ++len;
       --ptr;
   }

   if(ptr<path) //不存在路径分隔符
     ptr=path;
   else //找到路径分隔符
     ptr++;

   if(len >=size)//输出缓存长度不足,则截断
       len = size-1;
   strncpy(basename, ptr, len);
   *(basename+len)='\0';

   return 0;
}

//功能: 连接目录名与文件名或目录,即path/name
//int os_path_join(char *path, char *name, char *new_path, int size)
//{
//    return -1;
//}

//功能: 判断path是文件
int os_path_isfile(char *path)
{
   struct stat info;
   int rc;
   memset(&info, 0, sizeof(info));
   rc=stat(path, &info);
   if(rc<0) return 0;

   return S_ISREG(info.st_mode) ? 1 : 0;
}

//功能: 判断path是目录
int os_path_isdir(char *path)
{
   struct stat info;
   int rc;
   memset(&info, 0, sizeof(info));
   rc=stat(path, &info);
   if(rc<0) return 0;

   return S_ISDIR(info.st_mode) ? 1 : 0;
}

//功能: 判断path是绝对路径, 1-绝对路径; 0-不是绝对路径;
int os_path_isabs(char *path)
{
    if( (NULL==path) || (*path=='\0') ) return 0;

    //第一个字符是路径的分隔符,则路径是绝对路径,如"/root/home/"
    if(os_path_is_sep_char(*path)) return 1;

    //window的绝对路径格式如"c:/software/"等格式
    //判断方式: 第1个字符是字母,第2个字符是':'
    if(strlen(path)>=2)
    {
      char ch=toupper(*path);
      return ( (ch>='A' && ch<='Z') && (':'==*(path+1)) ) ? 1 : 0;
    }
    
    return 0;
}


//功能: 获取文件小;  注: 需要验证文件不存在的情况;
int os_path_file_getsize(char *path)
{
   int rc=0;
   struct stat stat_info;
   if(NULL==path) return -1;
   memset(&stat_info, 0, sizeof(stat_info));
   rc=stat(path,&stat_info);
   return stat_info.st_size;
}

//功能: 创建目录(递归创建目录)
//返回值:
//    0-成功; <0-失败;
int os_path_mkdir(char *path)
{
   char sub_path[NAME_MAX+1]={0};

   char *ptr=path;
   char *sub_ptr=sub_path;
   if((NULL==path) || ('\0'==*path)) return -1;

   for(;;)//while((*sub_ptr=*ptr)!='\0')
   {
      *sub_ptr=*ptr;
      if(os_path_is_sep_char(*sub_ptr) || ('\0'==*sub_ptr))//找到一个分隔符,到一级目录,则校验目录是否存在,不存在则创建;
      {
         if(!os_path_exists(sub_path))//不存在,则创建目录
         {
             *sub_ptr='/';//全部改为linux的分隔符
             if(OS_MKDIR(sub_path)!=0)//创建失败;
                return -2;
         }
      }
      if('\0'==*sub_ptr)//完成
          break;

      ++ptr;
      ++sub_ptr;
   }

   return 0;
}


//功能: 删除目录(如果目录下有子目录和文件),注意:该函数慎用
//int os_path_force_removedir(char *path)
//{
//  return -1;
//}

