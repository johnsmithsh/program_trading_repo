#ifndef __MXX_OS_TIME_H_
#define __MXX_OS_TIME_H_

#include <sys/time.h> //gettimeofday
#include <time.h>

/*
 * struct timespec
 * {
 * time_t tv_sec; //秒
 * long tv_nsec; /纳秒
 * };
 */
/**
 * @brief 获取系统运行时间(即系统启动时间开始计时的时间),用于测量时间
 * 
 * @note
 *    测量时间尽量不要使用gettimeofday,推荐使用clock_gettime
 **/
inline void os_lapsetime(struct timespec *ts)
{
    //struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, ts);
	//return *ts;
}

inline void os_gettimeofday(struct timespec *ts)
{
	struct timeval tv;
	tv.tv_sec  = 0;
	tv.tv_usec = 0;
	gettimeofday(&tv, 0);

	ts->tv_sec = tv.tv_sec;
	tv.tv_usec = tv.tv_usec * 1000;
	return;
}

/**
 * @brief 计算两个时间差, 单位微秒; 
 * 
 * @note
 **/
inline long difftime_to_macrosecond(struct timespec *t1, struct timespec *t2)
{
    return (t2->tv_sec -t1->tv_sec)*1000000+(long)((t2->tv_nsec-t1->tv_nsec)/1000);
}

/**
 * @brief 计算两个时间差, 单位毫秒; 
 * 
 * @note
 **/
inline long difftime_to_millisecond(struct timespec *t1, struct timespec *t2)
{
    return (t2->tv_sec -t1->tv_sec)*1000+(long)((t2->tv_nsec-t1->tv_nsec)/1000000);
}

/**
 * @brief 计算两个时间差, 单位秒; 
 * 
 * @note
 **/
inline long difftime_second(struct timespec *t1, struct timespec *t2)
{
	return t2->tv_sec -t1->tv_sec;
}

//#include <time.h> //localtime

//定义日期
typedef struct _st_os_date
{
   int year;  //公元纪年
   int month; //月份,1~12
   int day;  //天 1~31
   int weekday;//按周 1~7
   int yearday;//一年中的第几天; 0~365
}ST_OS_DATE;

//定义时间
typedef struct _st_os_time
{
   int hour;//时,0~23
   int minute;//分, 0~59
   int second;//秒,, 0~59
   int millisecond;//毫秒,0~999  1毫秒=0.001秒
   int macroseconds;//微秒,0~999 1微妙=0.001毫秒
}ST_OS_TIME;

//定义日期时间类型
typedef struct _st_os_date_time
{
    ST_OS_DATE date;
    ST_OS_TIME time;
}ST_OS_DATE_TIME;

//功能: 将struct tm时间设置为公历纪年
inline ST_OS_DATE * tm_to_os_date(struct tm *tm, ST_OS_DATE *os_date)
{
    if(NULL==os_date) return NULL;

    os_date->year=1900+tm->tm_year;
    os_date->month=tm->tm_mon+1;
    os_date->day=tm->tm_mday;
    os_date->weekday = tm->tm_wday;
    os_date->yearday = tm->tm_yday;
    return os_date;
}

inline ST_OS_TIME * tm_to_os_time(struct tm *tm, ST_OS_TIME *os_time)
{
   if(NULL==os_time) return NULL;
   os_time->hour=tm->tm_hour;
   os_time->minute=tm->tm_min;
   os_time->second=tm->tm_sec;
   os_time->millisecond=0;
   os_time->macroseconds=0;
   return os_time;
}

inline ST_OS_DATE * timeval_to_os_date(struct timeval *tv, ST_OS_DATE *os_date)
{
    if(NULL==os_date) return NULL;
    
    struct tm tm;
    localtime_r(&(tv->tv_sec), &tm);
    return tm_to_os_date(&tm, os_date);
}

inline ST_OS_TIME * timeval_to_os_time(struct timeval *tv, ST_OS_TIME *os_time)
{
   if(NULL==os_time) return NULL;

   struct tm tm;
   localtime_r(&(tv->tv_sec), &tm);
   tm_to_os_time(&tm, os_time);
   os_time->millisecond=(int)tv->tv_usec/1000;//毫秒
   os_time->macroseconds=tv->tv_usec%1000; //微妙
   return os_time;
}

//注: 使用getimeofday不是好主意,它需要进行用户/内核态切换;
inline ST_OS_DATE *os_get_date(ST_OS_DATE *os_date)
{
    struct timeval tv;

    if(NULL==os_date) return NULL;

    gettimeofday(&tv, 0);
    return timeval_to_os_date(&tv, os_date);
}

//注: 使用getimeofday不是好主意,它需要进行用户/内核态切换;
inline ST_OS_TIME *os_get_time(ST_OS_TIME *os_time)
{
    struct timeval tv;

    if(NULL==os_time) return NULL;

    gettimeofday(&tv, 0);
    return timeval_to_os_time(&tv, os_time);
}

//注: 使用getimeofday不是好主意,它需要进行用户/内核态切换;
inline ST_OS_DATE_TIME* os_get_date_time(ST_OS_DATE_TIME *os_date_time)
{
    struct timeval tv;

    if(NULL==os_date_time) return NULL;

    gettimeofday(&tv, 0);
    timeval_to_os_time(&tv, &(os_date_time->time));
    timeval_to_os_date(&tv, &(os_date_time->date));
    return os_date_time;
}

//获取始终的tickcount
inline long os_get_tickcount();

//获取当前时间; gettiemofday会进行用户/内核态切换
inline int os_get_timeval(struct timeval *tv)
{
   if(NULL==tv) return 0;
   gettimeofday(tv, 0);
   return 0;
}

//功能:计算两个时间差; 返回值: 时间差,单位:秒,精度:毫秒;
inline double os_diff_timeval(struct timeval tv1, struct timeval tv2)
{
  return (tv1.tv_sec-tv2.tv_sec)+(tv1.tv_usec-tv2.tv_usec)/1000000;
}


#endif
