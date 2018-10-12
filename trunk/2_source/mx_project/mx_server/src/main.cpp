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
  #include <signal.h>
#endif

#include "os_thread.h"
#include "ipc_shm_sysv.h"

#include "logfile.h"
#include "global_ctrl.h"
#include "servermanage.h"

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


//-------------------------------------------------------------
// 定义全局控制信息
//-------------------------------------------------------------
IpcShmSysV g_ctrl_shm;//!< 还是暂时不使用指针
st_shm_pubctrl *g_global_ctrl=NULL; //!< 全局控制信息,控制所有进程; 该结构在共享内存中,对所有进程开放
/**
 * @brief 获取服务器控制信息,该信息多个进程可以看到
 * @param 无
 * @retval 返回实例指针; NULL-创建实例失败
 * @note 单实例模式
 */
st_shm_pubctrl * create_global_ctrl_instance()
{
    int rc;
    char shm_name[]="./ctrl";
    
    //创建共享内存...
    rc=g_ctrl_shm.shm_open(shm_name, 0, sizeof(st_shm_pubctrl));/*根据文件生成key*/
    if(rc<0)
    {
        FATAL_MSG("Fatal: 打开共享内存失败,shmname=[%s], rc=[%d]", shm_name, rc);
        return NULL;
    }
    //共享内存关联到g_global_ctrl...
    if(NULL==g_global_ctrl)
    {
        g_global_ctrl=(st_shm_pubctrl *)g_ctrl_shm.get_shmaddr();//共享内存管理到全局变量
    }
    if(NULL==g_global_ctrl)
    {
        FATAL_MSG("Fatal: 共享内存关联到g_global_ctrl失败!,shmname=[%s]", shm_name);
        g_ctrl_shm.shm_close();
        return NULL;        
    }
    
    return g_global_ctrl;
}
//@brief 删除服务器控制实例
bool delete_global_ctrl_instance()
{
    g_ctrl_shm.shm_close();
    g_global_ctrl=NULL;
    return true;
}
//@brief 返回服务器控制实例指针
st_shm_pubctrl *get_global_ctrl_instance()
{
    return g_global_ctrl;
}
//-------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------
CServerManage g_server_manage;

/////////////////////////////////////////////////////////////////////////////////////////////


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
    //创建实例
    st_shm_pubctrl* global_ctrl_ptr=create_global_ctrl_instance();
    if(NULL==global_ctrl_ptr)
    {
        FATAL_MSG("%s", "FATAL: Cannot open global ctrl struct!\n");
        return;
    }
    
    global_ctrl_ptr=get_global_ctrl_instance();
    if(NULL==global_ctrl_ptr)
    {
        printf("Fatal: open global ctrl failed!\n");
        return;
    }
    
    global_ctrl_ptr->exit_code=EXITCODE_TRUE;
    printf("Warning: system is going to stop!\n");
    
    delete_global_ctrl_instance();
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
int set_signal_function();
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
    int rc;
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
    
    if(argc>1)
    {
        if(0==strcmp(argv[1], "stop"))
        {
            toStop();
            return 0;
        }
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
    
    //信号处理函数...
    set_signal_function();

    

    
    rc=g_server_manage.start_service();
    if(rc<0)
    {
        FATAL_MSG("启动服务失败!");
        delete_global_ctrl_instance();//!<其实此处省略也没有影响
        mxx_log_destroy(); //!< 此处省略也没有影响
        return -1;
    }
    //主进程循环
    while(EXITCODE_TRUE!=global_ctrl_ptr->exit_code)
    {
        os_thread_msleep(10);//休眠10毫秒
    }
    
    //退出程序前,清理资源...
    g_server_manage.stop_service();
    delete_global_ctrl_instance();
    mxx_log_destroy();
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// 设置系统信号处理函数
//--------------------------------------------------------------------------------------
void sig_term(int signo)
{
    st_shm_pubctrl *global_ctrl_ptr=get_global_ctrl_instance();
    if(NULL==global_ctrl_ptr)
        return;
    //INFO_MSG("收到信号:%d , 服务即将即将退出!",signo);   //信号处理函数使用记录日志,不知道是否存在问题;
    global_ctrl_ptr->exit_code=EXITCODE_TRUE;
}
int set_signal_function()
{
    /*
    处理SIGCHLD信号并不是必须的。
    如果父进程不等待子进程结束，子进程将成为僵尸进程（zombie）从而占用系统资源。
    如果父进程等待子进程结束，将增加父进程的负担，影响服务器进程的并发性能。
    在Linux下可以简单地将SIGCHLD信号的操作设为SIG_IGN。 
    signal(SIGCHLD,SIG_IGN); 
    */

    /* 
     * Solaris  Solaris缺省操作       Linux       Linux缺省操作 
     * SIGHUP   终止                 SIGHUP      忽略 
     * SIGINT   终止                 SIGINT      忽略 
     * SIGQUIT  终止，核心           SIGQUIT 终止，核心 
     * SIGILL   终止，核心           SIGILL 终止，核心 
     * SIGTRAP  终止，核心           SIGTRAP 忽略 
     * SIGABRT  终止，核心           SIGABRT 终止，核心 
     * SIGEMT   终止，核心           SIGEMT Linux 上不支持 
     * SIGFPE   终止，核心           SIGFPE 终止，核心 
     * SIGKILL  终止                 SIGKILL 终止 
     * SIGBUS   终止，核心           SIGBUS 终止，核心 
     * SIGSEGV  终止，核心           SIGSEGV 终止，核心 
     * SIGSYS   终止，核心           SIGSYS Linux 上不支持 
     * SIGPIPE  终止                 SIGPIPE 忽略 
     * SIGALRM  终止                 SIGALRM 忽略 
     * SIGTERM  终止                 SIGTERM 终止 
     * SIGUSR1  终止                 SIGUSR1 忽略 
     * SIGUSR2  终止                 SIGUSR2 忽略 
     * SIGCHLD  忽略                 SIGCHLD 忽略 
     * SIGPWR   忽略                 SIGPWR 忽略 
     * SIGWINCH 忽略                 SIGWINCH 进程停止 
     * SIGURG   忽略                 SIGURG 忽略 
     * SIGPOLL  终止                 SIGPOLL Linux 上不支持 
     * SIGSTOP  进程停止             SIGSTOP 进程停止 
     * SIGSTP   进程停止             SIGSTP 进程停止 
     * SIGCONT  忽略                 SIGCONT 忽略 
     * SIGTTIN  进程停止             SIGTTIN 进程停止 
     * SIGTTOU  进程停止             SIGTTOU 进程停止 
     * SIGVTALRM 终止                SIGVTALRM 终止，核心 
     * SIGPROF  终止                 SIGPROF 忽略 
     * SIGXCPU  终止，核心           SIGXCPU 终止，核心 
     * SIGXFSZ  终止，核心           SIGXFSZ 终止，核心 
     * SIGWAITING 忽略               SIGWAITING Linux 上不支持 
     * SIGLWP   忽略                 SIGLWP Linux 上不支持 
     * SIGFREEZE 忽略                SIGFREEZE Linux 上不支持 
     * SIGTHAW  忽略                 SIGTHAW Linux 上不支持 
     * SIGCANCEL 忽略                SIGCANCEL Linux 上不支持 
     * SIGRTMIN 终止                 SIGRTMIN Linux 上不支持 
     * SIGRTMAX 终止                 SIGRTMAX Linux 上不支持 
    */
    signal(SIGPIPE, SIG_IGN);//!< SIGPIPE关闭;unix系统中,send在等待协议传送时连接断开,send进程还会收到SIGPIPE信号;
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, sig_term);
    
    signal(SIGALRM, SIG_IGN);
    
    return 0;
}
//--------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////
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
 