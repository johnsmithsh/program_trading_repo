/*
//
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
#ifndef _MXX_LOGFILE_H_
#define _MXX_LOGFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

//功能: 初始化日志
//  一个进程中,只能调用一次
//参数:
//  @param [in]cfg_file: 配置文件(包含路径)
//返回值:
//   true-成功; false-失败;
bool mxx_log_init(const char *cfg_file);

//功能: 销毁日志
//  一个进程中,只能调用一次
bool mxx_log_destroy();

////////////////////////////////////////////////////
int mxx_log_level();

//@brief 输出格式化日志
//@param:
//  [in]level:日志级别,见LOG_LEVEL_xxx定义;
bool mxx_log_msg(int level, const char *msg_fmt,...);

//@brief debug日志
bool mxx_log_msg_debug(const char *msg_fmt,...);
//@brief info日志
bool mxx_log_msg_info(const char *msg_fmt,...);
//@brief warn日志
bool mxx_log_msg_warn(const char *msg_fmt,...);
//@brief error日志
bool mxx_log_msg_error(const char *msg_fmt,...);
//@brief fatal日志
bool mxx_log_msg_fatal(const char *msg_fmt,...);

#ifdef __cplusplus
}
#endif

#define MAX_SIZE_LOG_FILE  1024*1024*1024 //!< 日志文件大小上限

//定义日志消息级别
#define LOG_LEVEL_ALL   0
#define LOG_LEVEL_DEBUG 2000  //!< debug级别
#define LOG_LEVEL_INFO  4000  //!< info级别
#define LOG_LEVEL_WARN  6000  //!< warn级别
#define LOG_LEVEL_ERROR 7000  //!< error级别
#define LOG_LEVEL_FATAL 8000  //!< fatal级别


#if defined(_MSC_VER) && (_MSC_VER <= 1200) //!< vc平台
//宏定义: 输出各种级别日志
#define DEBUG_MSG mxx_log_msg_debug
#define WARN_MSG  mxx_log_msg_warn
#define INFO_MSG  mxx_log_msg_info
#define ERROR_MSG mxx_log_msg_error
#define ERROR_MSG mxx_log_msg_fatal

#else //!< linux

//宏定义: 输出各种级别日志;
//@note fmt_msg必须是格式化字符串 && 不能是字符串指针或变量
#define DEBUG_MSG(fmt_msg,...)  (LOG_LEVEL_DEBUG>=mxx_log_level()) && mxx_log_msg(LOG_LEVEL_DEBUG,"[%s:%d]"fmt_msg"\n",__FILE__,__LINE__,##__VA_ARGS__)
#define WARN_MSG(fmt_msg,...)   (LOG_LEVEL_WARN >=mxx_log_level()) && mxx_log_msg(LOG_LEVEL_WARN,"[%s:%d]"fmt_msg"\n",__FILE__,__LINE__,##__VA_ARGS__)
#define INFO_MSG(fmt_msg,...)   (LOG_LEVEL_INFO >=mxx_log_level()) && mxx_log_msg(LOG_LEVEL_INFO,"[%s:%d]"fmt_msg"\n",__FILE__,__LINE__,##__VA_ARGS__)
#define ERROR_MSG(fmt_msg,...)  (LOG_LEVEL_ERROR>=mxx_log_level()) && mxx_log_msg(LOG_LEVEL_ERROR,"[%s:%d]"fmt_msg"\n",__FILE__,__LINE__,##__VA_ARGS__)
#define FATAL_MSG(fmt_msg,...)  (LOG_LEVEL_FATAL>=mxx_log_level()) && mxx_log_msg(LOG_LEVEL_FATAL,"[%s:%d]"fmt_msg"\n",__FILE__,__LINE__,##__VA_ARGS__)

//@brief 向终端输出日志
#define SCREEN_MSG(level, fmt_msg,...) mxx_log_msg(level,"[%s:%d]"fmt_msg"\n",__FILE__,__LINE__,##__VA_ARGS__)
#endif

#endif
 