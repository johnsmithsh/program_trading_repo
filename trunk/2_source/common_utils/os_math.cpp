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

int main(int argc, char **argv)
{
    int rc;
    int intpart,fractpart;

    rc=os_double_split(123.4598, &intpart, &fractpart, 6);
}

