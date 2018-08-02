#ifndef __MDB_TRANSACTION_H_
#define __MDB_TRANSACTION_H_

/*
 * @desc
 * 由于fastdb一类的内存库不提供事务功能,只好自己实现.
 * 思路:
 * 1. begin_transaction创建事务队列
 * 2. insert/upadte/delete前保存记录
 * 3. rollback时根据事务队列回滚,commit则删除事务队列;
 */

#include <stack>
#include "mdb_transac_mem.h" //事务内存池相关结构

//数据表操作类型
#define DB_OPT_TYPE_INSERT '0' //insert操作
#define DB_OPT_TYPE_UPDATE '1' //update操作
#define DB_OPT_TYPE_DELETE '2' //delete操作

//表示一条数据记录
typedef struct __st_tb_record
{
    char db_name[64]; //数据库名
    char tb_name[64]; //数据表名
    char opt_type;    //数据表操作类型
    unsigned int  record_uid;  //record唯一标记,一般内存库每条记录都有一个唯一标记,fastdb中使用oid表示
    void *data_ptr;   //record数据指针
    int  data_size;   //record数据大小
}ST_TABLE_RECORD;

//该类描述一个数据库事务
//用于记录一个事务中所有被更新的记录的原始数据,主要用于事务回滚
class CMdbTransaction
{
  public:
    CMdbTransaction();
    virtual ~CMdbTransaction();
  private:
    //还没想好内存池和分配空间的拷贝问题,暂时不支持这些了
    CMdbTransaction(CMdbTransaction &obj);
    CMdbTransaction &operator=(CMdbTransaction &obj);
  public:
    /*@brief 向事务中添加一个记录
     *@param
     *   [in]db_name:数据库名,即内存库名;
     *   [in]tb_name:数据表名
     *   [in]opt_type:数据库操作类型: insert/update/delete
     *   []
     */
    int push_record(const char *db_name, const char *tb_name, char opt_type, unsigned int record_uid, void *data_ptr, size_t data_size);
    ST_TABLE_RECORD * top_record() { return m_transac_record_stack.top(); }
    ST_TABLE_RECORD * pop_record();
    
    //事务回滚或提交后,就不再需要这些数据了,需要清理数据
    void clear_records();
  private:
    //不知道怎么想的,std::stack没有clear函数
    void clear_stack();
  private:
    std::stack<ST_TABLE_RECORD *> m_transac_record_stack;//事务涉及的所有记录
    CTransacMempool *m_mempool_ptr;//内存池,每个事物都有自己独立的内存池
};

int mdb_begin_transaction();
int mdb_commit_transaction();
int mdb_rollback_transaction();

#endif
