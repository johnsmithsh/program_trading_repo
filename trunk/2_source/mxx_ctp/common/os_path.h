#ifndef __OS_PATH_H__
#define __OS_PATH_H__
/*
 * 说明: 仿造python的os.path模块定义c语言常用的路径操作
 *
 * */
//#include <string.h>

//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <ctype.h> //toupper/tolower

//#include <limits.h>

#ifdef _WIN32
#define OS_MKDIR(a) _mkdir(a)
#else
#define OS_MKDIR(a) mkdir((a),0755)
#endif

//定义路径分隔符
#define PATH_SEP_CHAR_LINUX '/'
#define PATH_SEP_CHAR_WIN  '\\'

//判断字符是路径分隔符
#define os_path_is_sep_char(ch) ( (PATH_SEP_CHAR_LINUX==(ch)) || (PATH_SEP_CHAR_WIN==(ch)) )

//功能: 获取path表示的绝对路径
int os_path_abspath(char *path, char *abspath, int size);

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
int os_path_dirname(char *path, char *dirname, int size);


//功能: 获取path的最后一个目录或文件
//参数:
//   [in]path: 路径
//   [out]basename, size: 返回的名字和缓存区长度;
//         size不足,则返回字符串被截断
//返回值: 0-成功; <0失败
int os_path_basename(const char *path, char *basename, int size);

//功能: 连接目录名与文件名或目录,即path/name
int os_path_join(char *path, char *name, char *new_path, int size);

//功能: 判断path是文件
int os_path_isfile(char *path);

//功能: 判断path是目录
int os_path_isdir(char *path);

//功能: 判断path是绝对路径, 1-绝对路径; 0-不是绝对路径;
int os_path_isabs(char *path);

//功能: 判断path存在
#define os_path_exists(path) ((NULL!=path)&&(0==access(path, F_OK)))

//功能: 获取文件小;  注: 需要验证文件不存在的情况;
int os_path_file_getsize(char *path);

//功能: 创建目录(递归创建目录)
//返回值:
//    0-成功; <0-失败;
int os_path_mkdir(char *path);

//删除目录
#define os_path_removedir(path) rmdir((path))

//功能: 删除目录(如果目录下有子目录和文件),注意:该函数慎用
int os_path_force_removedir(char *path);

#endif
