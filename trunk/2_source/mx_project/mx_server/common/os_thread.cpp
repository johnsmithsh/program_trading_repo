
#ifdef WIN32
#else
  #include <unistd.h>
#endif

#include "os_thread.h"

#define MAX_SLEEP_US  500000 //!<每次最大休眠微秒数 1毫秒=1000微秒,1ms=1000us
#define MIN_SLEEP_US  100    //!<每次最小休眠微秒数 1毫秒=1000微秒,1ms=1000us
//功能: 当前线程休眠,单位:毫秒
void os_thread_msleep(unsigned int millisecond)
{
    //window Sleep(millisecond);
	
	//usleep
	//根据centos 6.8 该函数描述,取值范围[0,1000000],单位:macrosecond;微秒; 但该范围可以随着发布版改变;
	//   为安全起见,每次休眠范围减少一半,即[0, 500000]
	//该函数可以被signal中断,并返回-1,
	//   errno=EINTR-signal中断; EINVA-参数高于上限1000000;
	//   不同的linux发布版errno错误码存在差异;
	//可以考虑nanosleep,休眠单位:纳秒; 要求:cpu大于1GHZ,暂时永不到该精度;
	millisecond *=1000;// 毫秒=>微秒
	unsigned int us_interval=0;
	do
	{
	    if(millisecond>MAX_SLEEP_US)  us_interval=MAX_SLEEP_US;
		else                          us_interval=millisecond;
		
		millisecond -= us_interval;
		usleep(us_interval);//!< 暂时不考虑被signal中断的情况	    
	}
	while(millisecond>MIN_SLEEP_US);
}

