
#include "common.h"
#include "ConfigFile.h"

#include <string.h>
#include <stdlib.h>

//#include "Func.h"

char g_comment_str[4]="//"; //define comment flag
char* find_comment_str(char *buffer, int buffer_len)
{
   if((NULL==buffer) || (buffer_len<=0)) return NULL;
   char *comm_str=strstr(buffer, g_comment_str);
   if((NULL==comm_str) || (comm_str>=(buffer+buffer_len-1)))
      return NULL;

   return comm_str;
}
//find the first non-blank char, then return the pointer,NULL-表示不存在;
inline char *find_non_blank(char *buffer, int buffer_len)
{
    if( (NULL==buffer) || (buffer_len<=0) ) return buffer;

    //char *ptr_str=buffer;
    //while( (*ptr_str!=0) && (ptr_str<(buffer+buffer_len)) )
    //{
    //    if((' '!=*ptr_str) || ('\t'!=*ptr_str))
    //        return ptr_str;
    //    ++ptr_str;
    //}
    //return NULL;

    int num=strspn(buffer, " \t");
    if(num>=buffer_len) return NULL;
    return buffer+num;
}

inline char *rfind_non_blank(char *buffer, int buffer_len)
{
    if((NULL==buffer)||(buffer_len<=0)) return NULL;
    char *ptr_str=buffer+buffer_len-1;
    while( (ptr_str>=buffer)&&('\0'!=*ptr_str)&&(is_blank(*ptr_str)) )
        --ptr_str;

    if(ptr_str<buffer) return NULL;
    return ptr_str;
}

// check if the data is comment
int is_comment(char *buffer, int buffer_len)
{
    if((NULL==buffer) || (buffer_len<=0))
        return 0;
    //find the first non-blank char
    char *ptr_str=find_non_blank(buffer, buffer_len);
    if(NULL == ptr_str) return 0;

    if(0==strncmp(ptr_str, g_comment_str, strlen(g_comment_str)))
        return 1;
    else
        return 0;
}

int is_section(char *buffer, int buffer_len,
               char *sec_buff=NULL, int sec_size=0)
{
    if(NULL!=sec_buff) *sec_buff=0;
    if( (NULL==buffer) || (buffer_len<=0) ) return 0;

    //去掉回车符与换行符
    if('\n'==*(buffer+buffer_len-1)) --buffer_len;
    if('\r'==*(buffer+buffer_len-1)) --buffer_len;
    
    //找到起始位置和长度
    int len=buffer_len;
    char *beg_ptr_str=find_non_blank(buffer, buffer_len);
    if( (NULL==beg_ptr_str) || ('['!=*beg_ptr_str) ) return 0;
    len=buffer+buffer_len-beg_ptr_str;

    //找到行注释位置并计算节头长度
    char *comment_str=find_comment_str(beg_ptr_str, len);
    if(NULL!=comment_str) len = comment_str-beg_ptr_str;

    //找到节定义末尾字符位置
    char *end_ptr_str=rfind_non_blank(beg_ptr_str, len);
    if( (NULL==end_ptr_str) || (']'!=*end_ptr_str) ) return 0;

    if(end_ptr_str+1-beg_ptr_str<=2) //no section name
        return 0;
    
    if(NULL != sec_buff)
    {
        len = end_ptr_str-beg_ptr_str-1;
        len=(len>sec_size) ? sec_size : len;
        strncpy(sec_buff, beg_ptr_str+1, len);
        // trim(sec_buff); 
       *(sec_buff+len)='\0';
        
    }

    return 1;    
}

/**
 * 功能: 解析配置项中的键值对
 *
 * 参数:
 *    [in]buffer/buffer_len: 字符串指针与长度
 *    [out]key_buff: 返回解析到的key, key_len表示缓冲区大小
 *    [in]key_len:
 *    [out]value_buffer: 返回解析到的值, value_len表示缓冲区大小
 *    [in]value_len:
 *    [out]comment_buff: 返回解析到的注释, 可置空; comment_len表示缓冲区大小
 * 返回值:
 *    0-成功;<0-失败;
 * 备注:
 *     调用该函数前确保参数buffer中保存的不是section name和注释行
 *     key中不能包含注释"//"和'='
 *     value中不能包含"//"
 **/
int split_profile_value(char *buffer, int buffer_len,
                        char *key_buff, int key_size,
                        char *value_buff, int value_size,
                        char *comment_buff=NULL, int comment_size=0)
{
    if(NULL!=key_buff)     *key_buff=0;
    if(NULL!=value_buff)   *value_buff=0;
    if(NULL!=comment_buff) *comment_buff=0;

    if((NULL==buffer) || (0==buffer_len)) return -1;

    //去掉回车符与换行符
    if('\n'==*(buffer+buffer_len-1)) --buffer_len;
    if('\r'==*(buffer+buffer_len-1)) --buffer_len;

    //找到第一个不是空白字符的位置
    char *key_ptr=find_non_blank(buffer, buffer_len);
    if( (NULL==key_ptr) || (key_ptr>(buffer+buffer_len)))//表示空行
        return -5;

    //获取键值分割符'='的位置
    char *split_ptr=strchr(key_ptr, '=');
    if((NULL==split_ptr)||(split_ptr>(buffer+buffer_len)))//不存在分割符'='
        return -6;

    //获取行注释符位置
    char *comment_ptr=find_comment_str(buffer, buffer_len);
    
    //no key,then return error
    if((NULL==key_ptr)||(split_ptr==key_ptr))//表示空行或第一个字符是'='
        return -1;
    //if(NULL==split_ptr)//没有分割符'='
    //    return -2;
    //注释符在分割符前，等同于没有分割符
    if((NULL!=comment_ptr)&&(comment_ptr<=split_ptr))
        return -3;

    //计算去掉注释后的长度
    //int total_len = (NULL==comment_ptr) ? buffer_len : (comment_ptr-buffer);
    

    int len=0;
    //get key
    len = split_ptr-key_ptr;
    len = (len>key_size) ? key_size : len;//如果返回缓冲区过小,则必须被截断,不能溢出
    strncpy(key_buff, key_ptr, len);
    *(key_buff+len)=0;

    trim_blank(key_buff);
    if(strlen(key_buff)<=0)
        return -4;

    //get value
    if(NULL==split_ptr) //no value
        return 0;
    if(NULL!=comment_ptr) //have comment
        len=comment_ptr-split_ptr-1;
    else   //no comment
        len = buffer+buffer_len-split_ptr-1;
    len = (len>value_size) ? value_size : len;//防止返回缓冲区溢出
    strncpy(value_buff, split_ptr+1, len);
    *(value_buff+len)=0;
    trim_blank(value_buff);

    //get comment
    if((NULL != comment_buff) && (NULL!=comment_ptr))
    {
        len = buffer+buffer_len-comment_ptr;
        len = (len>comment_size) ? comment_size : len;
        strncpy(comment_buff, comment_ptr, len);
        *(comment_buff+len)=0;
    }
    return 0;
    
}

//////////////////////////////////////////////////////////////////////////
// ConfigFile类实现
// -----------------------------------------------------------------------
ConfigFile::ConfigFile()
{
    m_file_stream=NULL;
}

// -----------------------------------------------------------------------
ConfigFile::ConfigFile(const char *filename)
{
    //ConfigFile();
    m_file_stream=NULL;
    load_cfg_file(filename);
}

// -----------------------------------------------------------------------
ConfigFile::~ConfigFile()
{
   if(NULL!=m_file_stream)
   {
     fclose(m_file_stream);
     m_file_stream=NULL;
   }

}

// -----------------------------------------------------------------------
int ConfigFile::load_cfg_file(const char *filename)
{
	if (filename == NULL || *filename == '\0')
		return -1;

    if(NULL!=m_file_stream)
    {
       fclose(m_file_stream);
       m_file_stream=NULL;
    }
	FILE *pFile = fopen(filename, "rb");
    if(NULL==pFile)
       return -2;

    m_file_stream=pFile;
	
	return 0;
}


// -----------------------------------------------------------------------
char* ConfigFile::read_profile_string(const char *section_name, char *key_name, char *value_buff, int value_size)
{
    //清空返回缓存
    if(NULL!=value_buff) *value_buff=0;

    //校验输入参数
    if( (NULL==section_name) || (*section_name=='\0') )// no section name, then return;
        return NULL;
    if( (NULL==key_name) || (*key_name==0)) //no key
        return NULL;
    
    FILE *pFileStream=m_file_stream;

    if(NULL==pFileStream)
        return NULL;

    fseek(pFileStream, 0, SEEK_SET);//start the begining of file

    char sec_tmp[MAX_INI_SECTION_NAME_LEN];
    char key_tmp[MAX_INI_KEY_NAME_LEN];
    char value_tmp[MAX_INI_VALUE_LEN];

    bool sec_flag=false;
    int ini_type;
    while((ini_type=get_next_option(pFileStream, 
                  sec_tmp, sizeof(sec_tmp), 
                  key_tmp, sizeof(key_tmp), 
                  value_tmp, sizeof(value_tmp)))>=0)
    {
        //if(INI_TYPE_NONE==ini_type)//没有配置项了,退出即可
        //   break;

        //if((INI_TYPE_SECTION!=ini_type) && (INI_TYPE_KEY_VALUE!=ini_type))//不是节名,也不是键值对,则直接跳过
        //   continue;

        if(INI_TYPE_NONE==ini_type)//没有配置项了,退出即可
           break;
        else if(INI_TYPE_SECTION==ini_type)//是节名section
        {
            if(!sec_flag)
            {
                sec_flag=(0==strcmp(sec_tmp, section_name));//判断是否是指定的section
                continue;
            }
            else //跳出指定的节名
            {
                return NULL;//break;
            }
        }
        else if(INI_TYPE_KEY_VALUE==ini_type)//key=value配置项
        {
            if( (sec_flag)&&(0==strcmp(key_name,key_tmp)) )//已经找到指定的节名
            {
                strncpy(value_buff, value_tmp, value_size);
                *(value_buff+value_size-1)=0;
                return value_buff;
            }
        }
    }

    return NULL;
}

/**
 * 功能:读取下一个配置项
 * 参数:
 *    [in]fpstream:文件对象
 *    [out]section_buff
 *    [out]key_buff:
 *    [out]value_buff:
 * 返回值:
 *    <0-错误; 0-已经没有配置项了; >=返回配置项类型:见INI_TYPE_宏定义
 **/
int ConfigFile::get_next_option(FILE *fpstream,
                                 char *section_buff, int section_size,
                                 char *key_buff, int key_size,
                                 char *value_buff, int value_size)
{
    //clear out buffer
    if(NULL!=section_buff) *section_buff=0;
    if(NULL!=key_buff)     *key_buff=0;
    if(NULL!=value_buff)   *value_buff=0;

    //check the input parameter
    if(NULL==fpstream)
        return -1;
    
    FILE *pFileStream=m_file_stream;

    //fseek(pFileStream, 0, SEEK_SET);//start the begining of file

    //define variable for temp use
    char section_name[MAX_INI_SECTION_NAME_LEN];
    char key_name[MAX_INI_KEY_NAME_LEN];
    char opt_value[MAX_INI_VALUE_LEN];

    //define variable for file line
    char buff_line[MAX_INI_LINE_LEN];
    //bool sec_flag=false;
    while(fgets(buff_line, sizeof(buff_line), pFileStream)!=NULL)
    {
        //trim_right(buff_line, '\n');
        //trim_right(buff_line, '\r');
        //trim(buff_line);
        
        if(0!=is_comment(buff_line, strlen(buff_line)))//注释行,则继续读取下一行
            continue;

        //find a section,then store to the output parameter
        memset(section_name, 0, sizeof(section_name));
        if(0!=is_section(buff_line, strlen(buff_line), section_name, sizeof(section_name)-1))
        {
            strncpy(section_buff, section_name, section_size);
            *(section_buff+section_size-1)=0;
            return INI_TYPE_SECTION;//返回section name
        }

        //find a ini key-value option,
        memset(key_name, 0, sizeof(key_name));
        memset(opt_value, 0, sizeof(opt_value));
        if(split_profile_value(buff_line, strlen(buff_line),
                               key_name, sizeof(key_name)-1,
                               opt_value, sizeof(opt_value)-1,
                               NULL, 0)<0)
            continue;

        //copy key
        strncpy(key_buff, key_name, key_size);
        *(key_buff+key_size-1)=0;

        //copy value, then return;
        strncpy(value_buff, opt_value, value_size);
        *(value_buff+value_size-1)=0;
        return INI_TYPE_KEY_VALUE;//找到键值对
    }

    return INI_TYPE_NONE;//没有找到任何ini类型
}


int ConfigFile::read_string(const char *section_name, const char *key_name, char *value_buf, int value_size, const char *default_value/*=""*/)
{
    if((NULL==section_name) || ('\0'==*section_name)) //必须指定section name
        return -1;       

    if((NULL==key_name) || ('\0'==*key_name)) //必须指定key name
        return -2;

    if((NULL==value_buf) || (value_size<=0)) //需要指定返回缓冲区
        return -3;
    *value_buf='\0';
    
    char ini_opt_value[MAX_INI_VALUE_LEN]={0};
    
    char *res_ptr=read_profile_string((char *)section_name, (char *)key_name,ini_opt_value,sizeof(ini_opt_value));
    if((NULL==res_ptr) || ('\0'==*ini_opt_value)) //找不到配置项,则设置默认值
    {
        if(NULL!=default_value)
            strcpy(value_buf, default_value);
        else
            *value_buf='\0';
    }
    else //找到配置项
    {
        snprintf(value_buf, value_size, "%s", ini_opt_value);
    }

    return 0;
}

std::string ConfigFile::read_string(const char *section_name, const char *key_name, const char *default_value/*=""*/)
{
    std::string res_str("");
    if((NULL==section_name) || ('\0'==*section_name)) //必须指定section name
        return std::string((NULL!=default_value) ? default_value : "");       
    
    if((NULL==key_name) || ('\0'==*key_name)) //必须指定key name
        return std::string((NULL!=default_value) ? default_value : "");

    char ini_opt_value[MAX_INI_VALUE_LEN]={0};
    char *res_ptr=read_profile_string((char *)section_name, (char *)key_name,ini_opt_value,sizeof(ini_opt_value));
    if((NULL==res_ptr) || ('\0'==*ini_opt_value)) //找不到配置项,则设置默认值
    {
        return std::string((NULL!=default_value) ? default_value : "");
    }
    else //找到配置项
    {
        return std::string(ini_opt_value);
    }

}

char ConfigFile::read_char(const char *section_name, const char *key_name, char default_value/*=0*/)
{
    if((NULL==section_name) || ('\0'==*section_name)) //必须指定section name
        return default_value;       
    
    if((NULL==key_name) || ('\0'==*key_name)) //必须指定key name
        return default_value;

    //读取配置项并根据配置项设置返回
    char ini_opt_value[MAX_INI_VALUE_LEN]={0};
    char *res_ptr=read_profile_string((char *)section_name, (char *)key_name,ini_opt_value,sizeof(ini_opt_value));
    if((NULL==res_ptr) || ('\0'==*ini_opt_value)) //找不到配置项,则设置默认值
    {
        return default_value;
    }
    
    return *ini_opt_value;
}

int ConfigFile::read_int(const char *section_name, const char *key_name, int default_value/*=0*/)
{
    if((NULL==section_name) || ('\0'==*section_name)) //必须指定section name
        return default_value;       

    if((NULL==key_name) || ('\0'==*key_name)) //必须指定key name
        return default_value;

    //读取配置项并根据配置项设置返回
    char ini_opt_value[MAX_INI_VALUE_LEN]={0};
    char *res_ptr=read_profile_string((char *)section_name, (char *)key_name,ini_opt_value,sizeof(ini_opt_value));
    if((NULL==res_ptr) || ('\0'==*ini_opt_value)) //找不到配置项,则设置默认值
    {
        return default_value;
    }
    
    return atoi(ini_opt_value);
}

double ConfigFile::read_double(const char *section_name, const char *key_name, double default_value/*=0*/)
{
    if((NULL==section_name) || ('\0'==*section_name)) //必须指定section name
        return default_value;       

    if((NULL==key_name) || ('\0'==*key_name)) //必须指定key name
        return default_value;

    //读取配置项并根据配置项设置返回
    char ini_opt_value[MAX_INI_VALUE_LEN]={0};
    char *res_ptr=read_profile_string((char *)section_name, (char *)key_name,ini_opt_value,sizeof(ini_opt_value));
    if((NULL==res_ptr) || ('\0'==*ini_opt_value)) //找不到配置项,则设置默认值
    {
        return default_value;
    }
    
    return atof(ini_opt_value);
}


