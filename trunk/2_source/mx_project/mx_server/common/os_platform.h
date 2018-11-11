#ifndef __MXX_OS_PLATFORM_H_
#define __MXX_OS_PLATFORM_H_

//操作系统相关定义
#ifdef _WIN32 //windows平台
   #define __MXX_OS_WINDOW__
   #ifdef _WIN64
     #define MXX_OS_WIN64
   #else
     #define MXX_OS_WIN32
   #endif
#elif __linux__ //linux/__linux不符合POSIX,已废弃
    #define __MXX_OS_LINUX__  //linux操作系统
     //
    #ifdef _AIX  //AIX系统
       #define MXX_OS_VER_AIX        //linux发行版
       #define MXX_OS_VERSION "AIX"  //发行版本号
    #elif __FreeBSD__ //freeBSD
       #define MXX_OS_VER_FREEBSD       //linux发行版
       #define MXX_OS_VERSION "FreeBsd" //发行版本号
    #endif
#elif __unix__
    #define __MXX_OS_UNIX__ //unix系统
#endif


//编译器相关定义
#ifdef _MSC_VER  //VC编译器
    #define __MXX_CC_MSC__
    //#if _MSC_VER >=1000 // VC++4.0以上
    //#if _MSC_VER >=1100 // VC++5.0以上
    //#if _MSC_VER >=1200 // VC++6.0以上
    //#if _MSC_VER >=1300 // VC2003以上
    //#if _MSC_VER >=1400 // VC2005以上
#elif __GNUC__   //GNU编译器(即gcc)
    #define __MXX_CC_GUNC_
#elif __BORLANDC__ //borland c
    #define __MXX_CC_BORLAND__
#elif __CYGWIN__
    #define __MXX_CC_CYGWIN_
#elif __MINGW32__
    #define __MXX_CC_MINGW__
#endif


//cpu相关信息定义

/*
//GCC
//
//    #ifdef __GNUC__
//    #if __GNUC__ >= 3 // GCC3.0以上
//
//
//Visual C++
//
//    #ifdef _MSC_VER
//    #if _MSC_VER >=1000 // VC++4.0以上
//    #if _MSC_VER >=1100 // VC++5.0以上
//    #if _MSC_VER >=1200 // VC++6.0以上
//    #if _MSC_VER >=1300 // VC2003以上
//    #if _MSC_VER >=1400 // VC2005以上
//
//
//Borland C++
//
//    #ifdef __BORLANDC__
//
//
//Cygwin
//
//    #ifdef __CYGWIN__
//    #ifdef __CYGWIN32__    //
//
//
//
//MinGW
//
//    #ifdef __MINGW32__
//
//
//
//    操作系统
//
//Windows
//
//    #ifdef _WIN32    //32bit
//    #ifdef _WIN64    //64bit
//    #ifdef _WINDOWS     //图形界面程序
//    #ifdef _CONSOLE     //控制台程序
//    //Windows（95/98/Me/NT/2000/XP/Vista）和Windows CE都定义了
//    #if (WINVER >= 0x030a)     // Windows 3.1以上
//    #if (WINVER >= 0x0400)     // Windows 95/NT4.0以上
//    #if (WINVER >= 0x0410)     // Windows 98以上
//    #if (WINVER >= 0x0500)     // Windows Me/2000以上
//    #if (WINVER >= 0x0501)     // Windows XP以上
//    #if (WINVER >= 0x0600)     // Windows Vista以上
//    //_WIN32_WINNT 内核版本
//    #if (_WIN32_WINNT >= 0x0500) // Windows 2000以上
//    #if (_WIN32_WINNT >= 0x0501) // Windows XP以上
//    #if (_WIN32_WINNT >= 0x0600) // Windows Vista以上
//
//
//UNIX
//
//    #ifdef __unix
//    //or
//    #ifdef __unix__
//
//
//Linux
//
//    #ifdef __linux
//    //or
//    #ifdef __linux__
//
//
//FreeBSD
//
//    #ifdef __FreeBSD__
//
//
//NetBSD
//
//    #ifdef __NetBSD__

#endif
