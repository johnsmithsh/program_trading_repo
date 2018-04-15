
#ifndef __COMMON_H_FOX_MATRIX_
#define __COMMON_H_FOX_MATRIX_
#include <string.h>

//移除字符串左侧指定的字符, str必须可被修改
inline char *trim_left(char *str, char ch=' ')
{
   if((NULL==str)||('\0'==ch)) return str;

   char *pSrc,*pDst;
   pSrc=pDst=str;
  
   //找到第一个非指定字符的位置
   for(; (ch==*pSrc)&&('\0'!=*pSrc); ++pSrc);

   //左侧没有指定字符
   if(pSrc==pDst) return str;

   while((*pDst++=*pSrc++)!='\0');
   return str;
}

//移除字符串右侧指定的字符, str必须可被修改
inline char *trim_right(char *str, char ch=' ')
{
    if((NULL==str)||('\0'==ch)) return str;

   char *ptr=str+strlen(str)-1;
   while((ptr>=str)&&(ch==*ptr))//注意两个逻辑判断的顺序,不要弄反了
      *ptr--='\0';

   return str;
}
//移除字符串两侧指定的字符, str必须可被修改
inline char *trim(char *str, char ch=' ')
{
   if((NULL==str)||('\0'==ch)) return str;
   trim_left(str, ch);
   trim_right(str, ch);

   return str;   
}

inline int is_blank(char ch)
{
    return ((' '==ch) || ('\t'==ch)) ? 1 : 0;
}

//移除字符串左侧指定的空白字符(即' '和'\t'), str必须可被修改
inline char *trim_left_blank(char *str)
{
   if(NULL==str) return str;
    
   char *pDst=str;
   char *pSrc=NULL;
   int num=strspn(str, " \t");
   if(0==num) return str;

   pSrc=str+num;
   while((*pDst++=*pSrc++)!='\0');
   return str;
}
//移除字符串右侧指定的空白字符(即' '和'\t'), str必须可被修改
inline char *trim_right_blank(char *str)
{
  if(NULL==str) return str;

  char *ptr=str+strlen(str)-1;
  while((ptr>=str)&&(0!=is_blank(*ptr)))//顺序不要弄反了
      *ptr--='\0';

   return str;
}
//移除字符串两侧指定的空白字符(即' '和'\t'), str必须可被修改
inline char *trim_blank(char *str)
{
   if(NULL==str) return str;

   trim_left_blank(str);
   trim_right_blank(str);

   return str;
}



#endif //__COMMON_H_FOX_MATRIX_

