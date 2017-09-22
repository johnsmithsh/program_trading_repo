#include "os_math.h"
#include <math.h>
#include <stddef.h>

int os_double_split(double d, int *intpart, int *fractpart, int precision)
{
   double __intpart=0.0;
   double __fractpart=0.0;
   __fractpart=modf(d, &__intpart);
   if(NULL!=intpart) *intpart=(int)__intpart;
   if(NULL!=fractpart) 
   {
     if(precision==0)
       *fractpart=0;
     else
       *fractpart=(int)__fractpart*pow(10, precision);
   }
   return (int)__intpart;
}

//四舍五入
double double_round(double d, double precision)
{
  int i=0;
  while((precision=precision*10)<=1.00000) 
    ++i;
  return ((int)(d*pow(10, i)+0.5))/pow(10,i);

  //return round(d/precision)*precision;
}

//整数部分;
int double_intpart(double d)
{
  double ipart=0.0;
  modf(d, &ipart);
  return (int)ipart;
}

//小数部分;
double double_fractpart(double d, double precision)
{
  double ipart=0.0;
  double fpart=0.0;
  fpart=modf(d, &ipart);

  return double_round(fpart, precision);
}

//浮点数比较
int double_compare(double d1, double d2, double precision)
{
   if((d1-d2)>precision) return 1;
   else if((d1-d2)<-precision) return -1;
   return 0;
}
