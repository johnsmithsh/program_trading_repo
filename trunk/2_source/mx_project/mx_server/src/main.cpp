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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#else
  #include <unistd.h>
#endif

#include "os_thread.h"

#include "logfile.h"
#include "global_ctrl.h"

/////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------
// 系统级参数宏定义
//-----------------------------------------------------
//前后台运行模式
#define DAEMON_MODE_FRONT '0' //!< 前台运行模式
#define DAEMON_MODE_BACK '1'  //!< 后台运行模式

//多实例模式
#define MULTI_INSTANCE_SINGTON_SYS_LEVEL       '0' //!< 运行单个实例
#define MULTI_INSTANCE_SINGTON_CUR_LEVEL       '1' //!< 当前部署目录单实例模式
#define MULTI_INSTANCE_MULTI                   '2' //!< 多实例模式


//系统级参数
//与服务无关,不会随着不发发生变化
typedef struct __sysPara
{
    char daemon_mode;         //!< 后台运行标记; '1'-后台运行; 其他-前台运行;
	char multi_instance_mode; //!< 单实例模式; '0'-运行单个实例; '1'-当前部署目录单实例模式; '2'-多实例模式;
}ST_SystemPara;
ST_SystemPara g_system_para={
                             DAEMON_MODE_FRONT,   //!< 前台运行
                             MULTI_INSTANCE_MULTI //!< 多实例模式
                            };
ST_SystemPara *get_system_para_instance()
{
    return &g_system_para;
}
///////////////////////////////////////////////////////////////////////////////////

char *get_build_info(char *buff, unsigned buff_size)
{
    snprintf(buff, buff_size-1, "build version:\n"
	        "         timestamp:%s %s\n", __DATE__,__TIME__
		   );
	return buff;
}
//功能:打印版本信息
void print_buildversion()
{
    char build_info_str[512]={0};
	get_build_info(build_info_str, sizeof(build_info_str));
    printf(build_info_str);
}

//功能: 打印帮助信息
void print_helpinfo(char *module_name)
{
   printf("Usage:\n");
   printf("    %s -h|-help -v|--version \n",module_name);
   printf("    %s  -b -t  cfgfile\n",module_name);
   printf("Description:\n"
          "    -h|--help  print help info\n"
          "    -v|--version print version information and build information\n"
          "    -b  background execute mode\n"
		 );
}

//@brief 判断运行模式实例
bool check_instance_mode()
{
    return false;
}

st_shm_pubctrl *g_global_ctrl=NULL; //!< 全局控制信息,控制所有进程; 该结构在共享内存中,对所有进程开放
//@brief 获取服务器控制信息,该信息多个进程可以看到
st_shm_pubctrl * create_global_ctrl_instance()
{
    //todo 创建共享内存...
	//todo 共享内存关联到g_global_ctrl...
	
    if(NULL==g_global_ctrl)
	{
	     g_global_ctrl=new st_shm_pubctrl();
	}
	
	
    return g_global_ctrl;
}
st_shm_pubctrl * open_global_ctrl_instance()
{
    return NULL;
}

bool delete_global_ctrl_instance()
{
    return false;
}
st_shm_pubctrl *get_global_ctrl_instance()
{
	return g_global_ctrl;
}


//@brief 功能: 后台运行
bool daemon_run()
{
    pid_t pid;
	pid=fork();
	if(pid<0)//fork failed
	{
	    printf("Error: daemon_run fork failed! errno=[%d]\n", errno);
		return false;
	}
	if(0==pid)//子进程
	{
	    return true;
	}
	else//父进程
	{
	    exit(0);//退出
	    return true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------
// 命令行参数处理
//-----------------------------------------------------

//@brief 功能: 停止服务
void toStop()
{
    st_shm_pubctrl* global_ctrl_ptr=get_global_ctrl_instance();
	if(NULL==global_ctrl_ptr)
	{
	    printf("Fatal: open global ctrl failed!\n");
		return;
	}
	
	global_ctrl_ptr->exit_code=EXITCODE_TRUE;
	printf("Warning: system is going to stop!\n");
	return;
}

//@brief 功能: 显示服务信息
void toShowInfo()
{
    st_shm_pubctrl* global_ctrl_ptr=get_global_ctrl_instance();
	if(NULL==global_ctrl_ptr)
	{
	    printf("Fatal: open global ctrl failed!\n");
		return;
	}
	
	//解析时间
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	localtime_r(&global_ctrl_ptr->main_start_time, &tm);//解析时间
	tm.tm_year +=1900;
	tm.tm_mon  += 1;
	//tm.tm_mday += 0;
	
	
	printf("main_pid=[%d]", global_ctrl_ptr->main_pid);
	printf("start time=[%lu] %04d%02d%02d %02d:%02d:%02d]\n", global_ctrl_ptr->main_start_time, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	//todo 其他统计信息
}

////////////////////////////////////////////////////////////////////////////////////
/** 
 * @brief 程序入口main函数
 * @brief 函数简要说明-测试函数
 * @param [in]argc  参数个数
 * @param [in]argv  参数数组 char** 
 *
 * @return 
 *        0:成功; 
 *     其他:失败;
 */
int main(int argc, char **argv)
{
    print_buildversion();
	
	ST_SystemPara * sys_para_ptr=get_system_para_instance();
	//解析参数...
	for(int i =0; i<argc; i++)
	{
	     if((strcmp(argv[i], "-h")==0) || (strcmp(argv[i], "--help")==0))//<! 打印帮助信息
		 {
		     print_helpinfo(argv[0]);
			 exit(0);
		 }
		 else if((strcmp(argv[i], "-v")==0) || (strcmp(argv[i], "--version")==0))//版本信息
		 {
		     //print_buildversion();
			 exit(0);
		 }
		 else if((strcmp(argv[i], "-b")==0))//后台运行模式
		 {
		     sys_para_ptr->daemon_mode=DAEMON_MODE_BACK;
		 }
	}
	
	//初始化日志...
	//初始化日志尽量提前做
	if(!mxx_log_init("./log_cfg.ini"))
	{
	    printf("Error: 初始化日志信息出错!\n");
		exit(1);
	}
	else
	{
	    //打印构建信息
	    char build_info_str[512]={0};
	    get_build_info(build_info_str, sizeof(build_info_str));
	    INFO_MSG("%s",build_info_str);
	}
	
	//根据参数判断是否已经可继续运行...
	//如果单实例模式,则不能继续运行必须退出
	if( (MULTI_INSTANCE_SINGTON_SYS_LEVEL==sys_para_ptr->multi_instance_mode) 
	    ||(MULTI_INSTANCE_SINGTON_CUR_LEVEL==sys_para_ptr->multi_instance_mode)
	  )
	{
	    if(!check_instance_mode())
		{
		    return -1;
		}
	}
	
	//根据参数切换后台运行模式...
	if(DAEMON_MODE_BACK==sys_para_ptr->daemon_mode)
	{
	    daemon_run();//后台运行模式,会创建一个子进程并退出
	}
	
	//todo 创建共享内存,写入当前运行消息...
	st_shm_pubctrl* global_ctrl_ptr=create_global_ctrl_instance();
	if(NULL==global_ctrl_ptr)
	{
	    FATAL_MSG("%s", "FATAL: Cannot open global ctrl struct!\n");
		return -2;
	}
	else
	{
	    global_ctrl_ptr->exit_code=EXITCODE_FALSE;
		global_ctrl_ptr->main_pid=os_getpid();//获取当前进程id
		global_ctrl_ptr->main_start_time=time(NULL);
		global_ctrl_ptr->last_chg_time=global_ctrl_ptr->main_start_time;
	}
	
	//todo 信号处理函数...
	
	//主进程循环
	while(EXITCODE_TRUE!=global_ctrl_ptr->exit_code)
	{
	    os_thread_msleep(10);//休眠10毫秒
	}
	
	//退出程序前,清理资源...
	delete_global_ctrl_instance();
	mxx_log_destroy();
    return 0;
}


/** 函数注释规范说明
 * @brief 函数简要说明-测试函数
 * @param [in]argc  参数个数
 * @param [in]argv  参数数组 char** @see CTest
 *
 * @return 返回说明
 *        -<em>false</em> fail
 *        -<em>true</em> succeed
 */
 /*
 note：指定函数注意项事或重要的注解指令操作符
 note格式如下：
            @note 简要说明

 retval：指定函数返回值说明指令操作符。(注:更前面的return有点不同.这里是返回值说明)
 retval格式如下：
            @retval 返回值 简要说明
            
 pre：指定函数前置条件指令操作符
 pre格式如下：
            @pre 简要说明
                   
 par：指定扩展性说明指令操作符讲。（它一般跟code、endcode一起使用 ）
 par格式如下：
          @par 扩展名字
          
 code、endcode：指定
 code、endcode格式如下：
            @code
                简要说明(内容)
            @endcode

 see：指定参考信息。
 see格式如下：
            @see 简要参考内容
    
 deprecated：指定函数过时指令操作符。
 deprecated格式如下：
          @deprecated 简要说明　
 -：生成一个黑心圆.
    -#：指定按顺序标记。
    ::：指定连接函数功能。（注：空格和“:”有连接功能,但建议还是使用”::”。只对函数有用。）
    它们格式如下: (-和::例子前面有了,就介绍-#例子。)
            - 简要说明
            -# 简要说明
            ::函数名
 */
 
 /*
    //!< 行尾注释4. appledoc不支持会会忽略, doxygen 支持.
 */
 