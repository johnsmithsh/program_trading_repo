#include <stdio.h>
#include <stdint.h> //c99引入的标准c库文件, 定义跨平台数据结构
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#else
  #include<sys/time.h>
#endif

#include "logfile.h"
#include "logFileManage.h"

int g_log_level=0;

//@brief  功能: 获取日志时间戳,单位:毫秒
int64_t get_log_timestamp()
{
    struct timeval tv;
	//struct timezone tz;
	gettimeofday(&tv, NULL);
	int64_t ts=(int64_t)tv.tv_sec*1000 + tv.tv_usec/1000;
	
    return ts;
}

//@brief 功能: 格式化消息
void format_msg(char *msg_buff, unsigned int msg_buff_size, const char *msg_fmt,...)
{
    assert(NULL!=msg_buff);
	assert(0<msg_buff_size);
	
	//消息头添加时间戳
	//int64_t timestamp=get_log_timestamp();//获取时间戳
	//sprintf(msg_buff, "%dll ", timestamp); //64位整数
	//int len=strlen(msg_buff);

	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff,msg_buff_size-1, msg_fmt, va);
	va_end(va);
}

//@brief 功能: 构建日志行头
void format_log_head(char *msg_buff, unsigned int msg_buff_size, int level)
{
    assert(NULL!=msg_buff);
	assert(2<=msg_buff_size);
	
    struct timeval tv;
	gettimeofday(&tv, NULL);
	//获取时间戳:单位:毫秒
	int64_t ts=(int64_t)tv.tv_sec*1000 + tv.tv_usec/1000;
	
	//解析时间
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	localtime_r(&tv.tv_sec, &tm);//解析时间
	
	//int64_t timestamp=get_log_timestamp();//获取时间戳
    char fmt_str[]="%lld %02d:%02d:%02d.%03d Level%d -- ";//防止编译wairning "警告:不建议使用从字符串常量到char*转换"
    snprintf(msg_buff, msg_buff_size-1, fmt_str, ts, 
	       tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec/1000, 
		   level);
}

//@brief 功能: 构建日志行头
void format_log_head(char *msg_buff, unsigned int msg_buff_size, int level, const char *cppfile, int line_no)
{
    assert(NULL!=msg_buff);
	assert(2<=msg_buff_size);
	
    struct timeval tv;
	gettimeofday(&tv, NULL);
	//获取时间戳:单位:毫秒
	int64_t ts=(int64_t)tv.tv_sec*1000 + tv.tv_usec/1000;
	
	//解析时间
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	localtime_r(&tv.tv_sec, &tm);//解析时间
	
	//int64_t timestamp=get_log_timestamp();//获取时间戳
    char fmt_str[]="%lld %02d:%02d:%02d.%03d Level%d -- [%s:%d]";//防止编译wairning "警告:不建议使用从字符串常量到char*转换"
    snprintf(msg_buff, msg_buff_size-1, fmt_str, ts, 
	       tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec/1000, 
		   level, cppfile, line_no);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  初始化日志信息
 */
bool mxx_log_init(const char *cfg_file)
{
    CLogFileManage *logfile=CLogFileManage::CreateInstance(cfg_file);
	if(NULL==logfile)
	   return false;
	g_log_level=logfile->get_level();
	return true;
}

/**
 * @brief  销毁日志信息
 */
bool mxx_log_destroy()
{
    //CLogFileManage *logfile=CLogFileManage::getInstance(cfg_file);
	//if(NULL==logfile)
	//    return true;
	
    CLogFileManage::DeleteInstance();
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//功能: 获取日志级别
int mxx_log_level()
{
    return g_log_level;
}

//@brief 输出日志; 直接输出不添加任何日志头;
bool mxx_log_msg(const char *msg_str)
{
    CLogFileManage *logfile=CLogFileManage::GetInstance();
	if(NULL==logfile) return false;
    
    return logfile->write_msg(LOG_LEVEL_ALL, msg_str);
}

//@brief 输出日志; 添加日志头"时间戳(毫秒) 时间 level%d -- [文件名:行号] "
bool mxx_log_msg(const char *cppfile, int line_no, int level, const char *mst_str)
{
    CLogFileManage *logfile=CLogFileManage::GetInstance();
	if(NULL==logfile) return false;
	
	//判断日志级别
	if(level < logfile->get_level()) return false;	
	
	char msg_buff[8192]={0};
	//添加消息头
	format_log_head(msg_buff, sizeof(msg_buff)-1, level, cppfile, line_no);
	int len=strlen(msg_buff);
	
	//格式化消息
	snprintf(msg_buff+len,sizeof(msg_buff)-len-1, "[%s:%d] %s", cppfile, line_no, mst_str);
	
	printf(msg_buff);
	
	return logfile->write_msg(level, msg_buff);
}

//功能: 输出日志; 添加日志头"时间戳(毫秒) 时间 level%d -- "
bool mxx_log_msg(int level, const char *msg_fmt,...)
{
    CLogFileManage *logfile=CLogFileManage::GetInstance();
	if(NULL==logfile) return false;
	
	//判断日志级别
	if(level < logfile->get_level()) return false;	
	
	char msg_buff[8192]={0};
	//添加消息头
	format_log_head(msg_buff, sizeof(msg_buff)-1, level);
	int len=strlen(msg_buff);
	
	//格式化消息
	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff+len,sizeof(msg_buff)-len-1, msg_fmt, va);
	va_end(va);
	
	printf(msg_buff);
	
	return logfile->write_msg(level, msg_buff);
	
}

//功能: 输出debug级别日志
bool mxx_log_msg_debug(const char *msg_fmt,...)
{
    char msg_buff[8192]={0};	
    
	//格式化消息
	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff,sizeof(msg_buff)-1, msg_fmt, va);
	va_end(va);
	
    char fmt_str[]="DEBUG--%s";
	return mxx_log_msg(LOG_LEVEL_DEBUG, fmt_str,msg_fmt);
}
//功能: 输出warn级别日志
bool mxx_log_msg_warn(const char *msg_fmt,...)
{
    char msg_buff[8192]={0};	
	
	//格式化消息
	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff,sizeof(msg_buff)-1, msg_fmt, va);
	va_end(va);
	
    char fmt_str[]="WARN--%s";
	return mxx_log_msg(LOG_LEVEL_WARN, fmt_str,msg_fmt);
}
//功能: 输出info级别日志
bool mxx_log_msg_info(const char *msg_fmt,...)
{
    char msg_buff[8192]={0};	
	
	//格式化消息
	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff,sizeof(msg_buff)-1, msg_fmt, va);
	va_end(va);
	
    char fmt_str[]="INFO--%s";
	return mxx_log_msg(LOG_LEVEL_INFO, fmt_str,msg_fmt);
}

//功能: 输出error级别日志
bool mxx_log_msg_error(const char *msg_fmt,...)
{
    char msg_buff[8192]={0};	
	
	//格式化消息
	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff,sizeof(msg_buff)-1, msg_fmt, va);
	va_end(va);
	
    char fmt_str[]="ERROR--%s";
	return mxx_log_msg(LOG_LEVEL_ERROR, fmt_str,msg_fmt);
}

//功能: 输出fatal级别日志
bool mxx_log_msg_fatal(const char *msg_fmt,...)
{
    char msg_buff[8192]={0};	
	
	//格式化消息
	va_list va;
	va_start(va, msg_fmt);
	vsnprintf(msg_buff,sizeof(msg_buff)-1, msg_fmt, va);
	va_end(va);
	
    char fmt_str[]="FATAL--%s";
	return mxx_log_msg(LOG_LEVEL_ERROR, fmt_str,msg_fmt);
}



 