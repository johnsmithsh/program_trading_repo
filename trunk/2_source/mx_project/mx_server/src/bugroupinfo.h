
/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义一组服务,该组服务必须提供相同的功能; 用于描述下级服务组
 *@details 
 *Others:  
 *Function List:  
         1.………… 
         2.………… 
 *@note  
 *@history:  
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/
#ifndef _MXX_BU_GROUP_INFO_H_
#define _MXX_BU_GROUP_INFO_H_

#include <list>

#include "bufuncinfo.h"

//定义一组服务,记录该组服务支持的业务
class CBuGroupInfo
{
  public:
    CBuGroupInfo();
	CBuGroupInfo(int group_id);
	virtual ~CBuGroupInfo();
	
	void clear();//清理数据
  public:
    void        set_groupid(int group_id) { m_group_item.group_id=group_id;}
    int         get_groupid() const       { return m_group_item.group_id;  }
    const char *get_groupno()             { return (const char *)m_group_item.group_no;  }
	
	//@brief 设置业务组信息
	int set_group_item(char *group_no, char *group_name, char *prog_name);
	int set_group_item(ST_BUGROUP_ITEM &inGroupItem);
	void get_group_item(ST_BUGROUP_ITEM &outGroupItem);
	
	//@brief 注册业务功能
    int register_func(int group_id, char *func_id, char *func_desc);
    int register_func(ST_BUFUNC_DESC *func_item_ptr);
    //@brief 注销业务功能; 删除业务功能信息与统计信息;
    int unregister_func(int group_id, char *func_id);
    int unregister_func(char *func_id);
    //@brief 禁用业务功能; 只是禁用,业务功能信息仍然保留
    int forbid_func(char *func_id, bool bForbid=false);
	
	//CBuFuncList &get_func_list();
	
	//@brief 根据业务功能id找到对应的业务信息
	ST_BUFUNC_INFO * get_funcinfo(char *func_id);
	//@brief 判断该业务组是否支持id对应的业务功能
	bool check_support_func(char *func_id);
	
	//返回业务组下所有业务功能信息
	const TBuFuncList &list_all_func_info();
  private:
    //int m_group_id;//!< 服务组号
	ST_BUGROUP_ITEM m_group_item;//!< 业务组说明
	
  private:
    //CBuFuncList m_bufunc_info;
	TBuFuncList m_bufunc_info;//!< 该业务组支持的业务功能信息
};
//////////////////////////////////////////////////////////////////////////////////////////////////
//group列表
class CBuGroupInfoList
{
  private:
    CBuGroupInfoList();
	virtual ~CBuGroupInfoList();
	
	CBuGroupInfoList(CBuGroupInfoList const&);
	CBuGroupInfoList& operator=(CBuGroupInfoList const&);
	
  public:
    CBuGroupInfo * alloc();
	void free(CBuGroupInfo *group_info_ptr);
	void clear();
	
	
	CBuGroupInfo *find_groupinfo(int group_id);
	CBuGroupInfo *find_groupinfo(char * group_no);
	
	ST_BUFUNC_INFO *find_funcinfo(int group_id, char *func_id);
	int get_lists(std::list<CBuGroupInfo*> &groupinfo_list);
	std::list<CBuGroupInfo*> & get_lists();
	
	
  private:
    CBuGroupInfo m_group_vec[128];//!< 业务进程group信息数组
	//std::map<std::string, int> m_groupp_map;  //!< key-grou_no; value-group_id
	std::list<CBuGroupInfo*> m_free_groupinfo;//!< 未被使用的指针列表
	std::list<CBuGroupInfo*> m_use_groupinfo; //!< 已经被使用的指针列表
    static unsigned int m_max_group_id;

  //单实例: 饿汉模式(程序运行时初始化)问题:
  // 在两个单模式的构造函数中使用对方的的实例使用对方,由于初始化顺序无法保证,可能导致程序未运行就崩溃;
  //private:
  //  static CBuGroupInfoList m_instance
  //单实例:懒汉模式(堆),第一次运行被初始化
  //bug:
  //   程序崩溃,未进行del_instance,可能导致问题; 虽然linux进程退出会自动释放资源,单仍然是个隐患;  
  //private:
  //  static CBuGroupInfoList *m_instance;
  //  //static ItcMutex m_mutex_instance;//给单实例加锁有点不划算, 但是create_instance一定在多线程启动先调用,程序退出前调用del_instance
  //  //  注意: 双重检查锁定模式(DCLP)在编译器优化+多核模式下并不可靠; 推荐使用volatile  or c++11的内存屏障技术 or c++11的aotomic;
  //  //  见《http://blog.jobbole.com/86392/》
  public:
  //  ////注:没有加锁, 一定在多线程启动前调用一次
  //  //static CBuGroupInfoList *create_instance();
      //为了兼容
      static CBuGroupInfoList *get_instance() { return CBuGroupInfoList::instance(); }
  //  //注:为了性能没有加锁, 一定在程序退钱调用,多线程运行中禁止调用; 否则可能造成泄露
  //static CBuGroupInfoList *del_instance();
  
  private:
    //单实例-懒汉模式(局部静态变量),这个版本相对最佳
	//BUG:
	//  1. 返回的是指针, Instance() 的调用者很可能会误认为他要检查指针的有效性, 并负责销毁. 构造函数和拷贝构造函数也私有化了, 这样类的使用者不能自行实例化.
	//  2. 如果唯一实例尚未创建时, 有两个线程同时调用创建方法, 且它们均没有检测到唯一实例的存在, 便会同时各自创建一个实例, 这样就有两个实例被构造出来, 从而违反了单例模式中实例唯一的原则. 解决这个问题的办法是为指示类是否已经实例化的变量提供一个互斥锁 (虽然这样会降低效率).
	//局部静态变量模式,建议在多线程运行前获取一次该实例;
	//注: 不能进行删除操作
    static CBuGroupInfoList * instance()
	{
	    static CBuGroupInfoList theSingleton;
		return &theSingleton;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif
