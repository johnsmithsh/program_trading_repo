#ifndef MXX_RTTI_MACRO_H_
#define MXX_RTTI_MACRO_H_

/*
 * 定义数据结构struct运行时说明信息
 * 用于接口协议定义
 *
 *  根据fastdb的class.h改写
 *  为了在程序中同时使用fastdb与rtti,宏定义需要区分,最好有单独的命名空间,与fastdb宏定义区分开
 *
 **/
//定义rtti命名空间即相关宏定义
#define USE_NAMESPACES //默认使用命名空间
#ifdef USE_NAMESPACES 
#define BEGIN_RTTI_NAMESPACE namespace ns_rtti {
#define END_RTTI_NAMESPACE }
#define USE_RTTI_NAMESPACE using namespace ns_rtti;
#define RTTI_NS ns_rtti
#else //不使用命名空间
#define BEGIN_RTTI_NAMESPACE
#define END_RTTI_NAMESPACE
#define USE_RTTI_NAMESPACE 
#define RTTI_NS
#endif



#endif
