#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "logFileManage.h"
#include "ConfigFile.h"
#include "os_path.h"
#include "os_file.h"

#define LOG_CFG_FILE_NAME "../conf/log_cfg.ini"
#define DEFAULT_LOG_PATH "../log/"
//功能: 判断日志文件大小是否超出限制; true-超出限制; false-未超出限制;
bool check_logfile_size(const char *logfilename, unsigned int max_filesize);
bool get_next_logfilename(const char *dir_path, char *filename, unsigned int filenamesize, int max_filesize);
//功能: 判断日志文件名与当前系统日期是否匹配; true-匹配; false-不匹配;
bool check_logfile_name(const char *logfilename);

CLogFileManage* CLogFileManage::m_instance=NULL;

CLogFileManage* CLogFileManage::CreateInstance(const char *cfgfile)
{
    if(NULL==m_instance)
	{
	    m_instance=new CLogFileManage();
		if(!m_instance->init(cfgfile))
		{
		    delete m_instance;
			m_instance=NULL;
		}
	}
	return m_instance;
}

CLogFileManage* CLogFileManage::GetInstance()
{
    //if(NULL==m_instance)
	//{
	//    m_instance=new CLogFileManage();
	//	if(!m_instance->init())
	//	{
	//	    delete m_instance;
	//		m_instance=NULL;
	//	}
	//}
	return m_instance;    
}

void CLogFileManage::DeleteInstance()
{
    if(NULL != m_instance)
	{
	    delete m_instance;
		m_instance=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////
CLogFileManage::CLogFileManage():m_logfile(NULL),m_level(0)
{
    memset(m_cfg_file, 0, sizeof(m_cfg_file));
	memset(m_log_path, 0, sizeof(m_log_path));
	strcpy(m_log_path, "./");//默认当前路径
}

CLogFileManage::~CLogFileManage()
{
    memset(m_cfg_file, 0, sizeof(m_cfg_file));
    memset(m_log_path, 0, sizeof(m_log_path));
    if(NULL!=m_logfile)
	{
	    fclose((FILE*)m_logfile);//delete m_logfile;
		m_logfile=NULL;
	}
}



bool CLogFileManage::init(const char *cfgfile/*=NULL*/)
{
    //char cfg_file_name[256]={0};
	
	//更新配置文件成员变量
	if( (NULL!=cfgfile) && (strlen(cfgfile)>0) )
	    strncpy(m_cfg_file, cfgfile, sizeof(m_cfg_file)-1);
	
	if(strlen(m_cfg_file)<=0)
	    return false;
	
    //读取日志配置信息...
    ConfigFile cfgini(m_cfg_file);
	cfgini.read_string("log", "logpath", m_log_path, sizeof(m_log_path)-1, DEFAULT_LOG_PATH);//日志路径
	m_level=cfgini.read_int("log", "level", 0); //日志级别
	m_logfile_max_size=cfgini.read_int("log", "max_size", 1024*1024*1024); //日志文件最大大小
	
	//创建日志...
	if(NULL!=m_logfile)//如果存在,则删除原日志
	{
	    DelLogFileHandle(m_logfile);//delete m_logfile;
		m_logfile=NULL;
	}
	char filename[256]={0};
	get_next_logfilename(m_log_path, filename, sizeof(filename)-1,m_logfile_max_size);
	m_logfile=OpenLogFileHandle(filename);
	if(NULL==m_logfile)
	    return false;
	return true;
}

bool CLogFileManage::write_msg(int level, char *msg)
{
    if(level<m_level) 
	    return true;
	
    //判断日志文件大小超出上限,超出,则打开创建新日志
    {
	    char current_file_name[256];//获取当前日志文件名
		bool b_rc=check_logfile_size(current_file_name, m_logfile_max_size);//判断文件超出大小限制
		bool b_datematch=check_logfile_name(current_file_name);//判断文件名与系统日期是否匹配
		if(b_rc || (!b_datematch) || (NULL==m_logfile))//超过大小 && 日期不匹配 && 日志未打开,则关闭原日志文件,打开新的日志文件
		{
		    if(NULL!=m_logfile)//删除原日志
			{
    			DelLogFileHandle(m_logfile);//delete m_logfile;
	    	    m_logfile=NULL;
	        }
		    char logfilename[256]={0};
		    get_next_logfilename(m_log_path, logfilename, sizeof(logfilename)-1, m_logfile_max_size);
			m_logfile=OpenLogFileHandle(logfilename);
			if(NULL==m_logfile)//打开日志失败
			    return false;
		}
	}
	
	//输出日志
	write_to_file(msg);
	return true;    
}

LogFileHandle  CLogFileManage::OpenLogFileHandle(char *logfilename)
{
    FILE * fp=fopen(logfilename, "a+");
	if(NULL==fp) 
	    return NULL;
	return (void *)fp;
}

bool CLogFileManage::DelLogFileHandle(LogFileHandle LogFileHandle)
{
    if(NULL==LogFileHandle) return true;
	fclose((FILE*)LogFileHandle);
	return true;
}

int CLogFileManage::write_to_file(char *msg)
{
	if(NULL==msg) return -2;
	if(NULL==m_logfile) return -1;
	fwrite(msg, 1, strlen(msg), (FILE *)m_logfile);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//功能: 判断日志文件大小是否超出限制; true-超出限制; false-未超出限制;
bool check_logfile_size(const char *logfilename, unsigned int max_filesize)
{
	return (os_filesize(logfilename)>=(int)max_filesize);
}

//功能: 判断日志文件名与当前系统日期是否匹配; true-匹配; false-不匹配;
bool check_logfile_name(const char *logfilename)
{
    if( (NULL==logfilename) || (strlen(logfilename)<=0) )
	    return false;
		
	//获取当前日期
	char curr_date[16]={0};
	time_t t;
	struct tm tm;
	memset(&t, 0, sizeof(t));
	memset(&tm, 0, sizeof(tm));
	time(&t);
	localtime_r(&t, &tm);
	tm.tm_year +=1900;
	tm.tm_mon  += 1;
	//tm.tm_mday += 0;
	snprintf(curr_date, sizeof(curr_date)-1, "%04d%02d%02d", tm.tm_year, tm.tm_mon, tm.tm_mday);
	
	//获取日志文件名
	char filename[256]={0};
	os_path_basename(logfilename, filename, sizeof(filename));
	return 0==strncmp(filename, curr_date, strlen(curr_date));	
}

bool get_next_logfilename(const char *dir_path, char *filename, unsigned int filenamesize, int max_filesize)
{
    assert(NULL!=filename);
	assert(filenamesize>1);

    char logfilename[256]={0};
	
	time_t t;
	struct tm tm;
	memset(&t, 0, sizeof(t));
	memset(&tm, 0, sizeof(tm));
	
	time(&t);
	localtime_r(&t, &tm);
	
	tm.tm_year +=1900;
	tm.tm_mon  += 1;
	//tm.tm_mday += 0;
	
	bool b_exist_path=((NULL!=dir_path) && (strlen(dir_path)>0));//是否指定目录
    int i=0;
	for(i=0; ; i++)
	{
	    if(b_exist_path)
	        snprintf(logfilename, sizeof(logfilename)-1, "%s/%04d%02d%02d_%03d.log", dir_path, tm.tm_year, tm.tm_mon, tm.tm_mday, i);
		else
		    snprintf(logfilename, sizeof(logfilename)-1,"%04d%02d%02d_%03d.log", tm.tm_year, tm.tm_mon, tm.tm_mday, i);
		if(!check_logfile_size(logfilename, max_filesize))//没有超过大小,则终止循环
		    break;
	}
	
	memset(filename, 0, filenamesize);
	strncpy(filename,  logfilename, (filenamesize-1));
	return false;
}
