#ifndef __MXX_OS_MATH_H_
#define __MXX_OS_MATH_H_

#define DEFAULT_PRECISION  0.000001 //默认精度

//功能:拆分double,整数部分与小数部分
int os_double_split(double d, int *intpart, int *fractpart, int precision);

//功能: 获取整数部分
int    double_intpart(double d);
//功能: 获取小数部分

double double_fractpart(double d, double precision=DEFAULT_PRECISION);

//功能: 浮点数比较, 返回值: 1:d1>d2; 0:d1==d2; -1:d1<d2
int double_compare(double d1, double d2, double precision=DEFAULT_PRECISION);

//功能: double四舍五入
double double_round(double d, double precision=DEFAULT_PRECISION);

#endif

