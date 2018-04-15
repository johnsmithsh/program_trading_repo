/**
    ConfigFile.h
 ini

 Author:

 History:

 Remark:
   1. 配置文件说明
   注释符:  "//"
   section name特殊符号: '['、']'
   分割符:  '='
   section name、key、value不能包含特殊字符;

   2. 该实现每次都需要从头读取配置文件,故该实现不适合运行过程中频繁读取ini配置文件,只适合在初始化过程中初始化配置.

**/

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <string>
#include <stdio.h>
//////////////////////////////////////////////////////////////////////////

//定义section/key/value的长度,每行的最大长度
#define MAX_INI_SECTION_NAME_LEN 64   //section name 最大长度
#define MAX_INI_KEY_NAME_LEN     64   //key name最大长度
#define MAX_INI_VALUE_LEN        1024 //value最大长度
#define MAX_INI_LINE_LEN         2048 //ini配置文件每行最大长度

//定义ini配置项类型
#define INI_TYPE_NONE      0 //没有配置项
#define INI_TYPE_SECTION   1 //section type 节名
#define INI_TYPE_KEY       2 //key type   键
#define INI_TYPE_VALUE     3 //value type 值
#define INI_TYPE_COMMENT   4 //comment 注释
#define INI_TYPE_KEY_VALUE 5 //键值对

class ConfigFile
{
  public:
    ConfigFile();
    ConfigFile(const char *filename);
    ~ConfigFile();

  public:

    /*
     * 功能: 读取指定配置项
     * 参数:
     *    [in]section_name: ini节名
     *    [in]key_name: 键名
     *    [out]value_buf: 返回读取到的值,value_len表示缓冲区大小
     *    [in]default_value: 如果找不到指定的配置项,则返回该参数指定的默认值
     * 返回值:
     *    0-成功; <0-错误;
     */ 
    int         read_string(const char *section_name, const char *key_name, char *value_buf, int value_size, const char *default_value="");
    //功能: 读取指定配置项; 如果不存在,则返回空字符串;
    std::string read_string(const char *section_name, const char *key_name, const char *default_value="");
    //功能: 读取指定配置项
    char   read_char(const char *section_name,   const char *key_name, char   default_value=0);
    int    read_int(const char *section_name,    const char *key_name, int    default_value=0);
    double read_double(const char *section_name, const char *key_name, double default_value=0.0);
 public:

    //获取第一个配置项
    //int get_first_option(char *section_buff, int section_len, char *key_name, int key_len);
    //获取下一个配置项
    //int get_next_option(char *section_buff, int section_len, char *key_name, int key_len);
#ifdef __UNIT_TEST_FLAG__ //单元测试
 public:
#else
 private:
#endif
    int load_cfg_file(const char *filename);

    //读取指定的配置项,NULL找不到指定的配置项; 找到配置项,则返回参数value_buf
    char* read_profile_string(const char *section_name, char *key_name, char *value_buf, int value_size);

    /**
     * 功能:读文件取下一个配置项
     * 参数:
     *    [in]fpstream:文件对象
     *    [out]section_buff
     *    [out]key_buff:
     *    [out]value_buff:
     * 返回值:
     *    <0-错误; 0-已经没有配置项了; >=返回配置项类型:见INI_TYPE_宏定义 
     **/
     int get_next_option(FILE *fpstream,
                         char *section_buff, int section_size,
                         char *key_buff, int key_size,
                         char *value_buff, int value_size);

 public:
     FILE * get_filestream() { return m_file_stream; }
  private:
    char m_buf[4096];
    int m_err_code;

 private:
    char m_file_name[256];
    FILE *m_file_stream;//
};

#endif //__CONFIGFILE_H__
