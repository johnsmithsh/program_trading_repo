/*
////
////                       _oo0oo_
////                      o8888888o
////                      88" . "88
////                      (| -_- |)
////                      0\  =  /0
////                    ___/`---'\___
////                  .' \\|     |// '.
////                 / \\|||  :  |||// \
////                / _||||| -:- |||||- \
////               |   | \\\  -  /// |   |
////               | \_|  ''\---/''  |_/ |
////               \  .-\__  '-'  ___/-. /
////             ___'. .'  /--.--\  `. .'___
////          ."" '<  `.___\_<|>_/___.' >' "".
////         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
////         \  \ `_.   \_ __\ /__ _/   .-` /  /
////     =====`-.____`.___ \_____/___.-`___.-'=====
////                       `=---='
////
////
////     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////
////    Beg to all gods in the sky    No Bug forever
////
////
////
*/
#ifndef _MXX_LOGFILEMANAGE_H_
#define _MXX_LOGFILEMANAGE_H_
#include <stdio.h>

typedef void*  LogFileHandle;
//@brief 日志文件管理
//  单实例模式
class CLogFileManage
{
protected:
    CLogFileManage();
	virtual ~CLogFileManage();
public:
    bool init(const char *cfgfile=NULL);
	bool write_msg(int level, char *msg);
	int get_level() { return m_level; }
public:
	LogFileHandle GetLogFileHandle() {  return m_logfile; }
private:
	LogFileHandle OpenLogFileHandle(char *logfilename);
	bool DelLogFileHandle(LogFileHandle LogFileHandle);
	int write_to_file(char *msg);
protected:
    LogFileHandle m_logfile;
	char m_cfg_file[256];//!< 配置文件路径
	char m_log_path[256];//!< 日志文件路径
	
	int m_level;             //!< 日志级别
	int m_logfile_max_size; //!< 日志文件最大大小
	char m_date[16];        //<!系统日期
	
	//int m_check_filesize_count;

public://单实例操作
    static CLogFileManage* CreateInstance(const char *cfgfile);
    static CLogFileManage* GetInstance();
	static void DeleteInstance();
protected:
    static CLogFileManage *m_instance;//单实例	
};

#endif
 
