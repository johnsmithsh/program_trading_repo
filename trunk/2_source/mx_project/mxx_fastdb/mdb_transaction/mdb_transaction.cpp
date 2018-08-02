#include <string.h>

#include "mdb_transaction.h"
          

CMdbTransaction::CMdbTransaction():m_mempool_ptr(NULL)
{
    //m_transac_record_stack.clear();
}

CMdbTransaction::~CMdbTransaction()
{
    clear_stack();//m_transac_record_stack.clear();
    if(NULL!=m_mempool_ptr)
    {
        delete m_mempool_ptr;
        m_mempool_ptr = NULL;
    }
}

//清空stack
void CMdbTransaction::clear_stack()
{
    //不知道与逐个pop相比,哪个高
    std::stack<ST_TABLE_RECORD *> empty_stack;
    swap(m_transac_record_stack, empty_stack);
}
/*@brief 向事务中添加一个记录
 *@param
 *   [in]db_name:数据库名,即内存库名;
 *   [in]tb_name:数据表名
 *   [in]opt_type:数据库操作类型: insert/update/delete
 *   []
 */
int CMdbTransaction::push_record(const char *db_name, const char *tb_name, char opt_type, unsigned int record_uid, void *data_ptr, size_t data_size)
{
    if((NULL==data_ptr)||(data_size==0))
        return -1000;
    //操作类型是否合法
    if((DB_OPT_TYPE_INSERT!=opt_type)&&(DB_OPT_TYPE_UPDATE!=opt_type)&&(DB_OPT_TYPE_DELETE!=opt_type))
        return -1001;
    
    //延迟分配内存,每个事务只会在第一次使用时调用
    if(NULL==m_mempool_ptr)
    {
        m_mempool_ptr = new CTransacMempool(MEMPOOL_INIT_SIZE);
        if(NULL==m_mempool_ptr)//分配内存失败
            return -2000;
    }
    
    //内存池申请空间
    ST_TABLE_RECORD *tb_record_ptr=(ST_TABLE_RECORD *)m_mempool_ptr->alloc(data_size+sizeof(ST_TABLE_RECORD));
    if(NULL==tb_record_ptr) //内存池分配内存失败了
        return -3000;
    //
    memset(tb_record_ptr, 0, sizeof(ST_TABLE_RECORD));
    strncpy(tb_record_ptr->db_name, db_name, sizeof(tb_record_ptr->db_name)-1);
    strncpy(tb_record_ptr->tb_name, tb_name, sizeof(tb_record_ptr->tb_name)-1);
    tb_record_ptr->record_uid = record_uid;
    tb_record_ptr->opt_type   = opt_type;
    tb_record_ptr->data_ptr   = (void *)tb_record_ptr+sizeof(ST_TABLE_RECORD);
    memcpy(tb_record_ptr->data_ptr, data_ptr, data_size);
    tb_record_ptr->data_size  = data_size;
    
    
    m_transac_record_stack.push(tb_record_ptr);//添加到队列中
    
    return 0;
}

ST_TABLE_RECORD * CMdbTransaction::pop_record()
{
    //if(NULL==pRecord)
    //    return NULL;
    if(m_transac_record_stack.empty())
        return NULL;
    ST_TABLE_RECORD * tb_record_ptr = m_transac_record_stack.top();
    m_transac_record_stack.pop();
    return tb_record_ptr;
}

void CMdbTransaction::clear_records()
{
    clear_stack();//m_transac_record_stack.clear();
    if(NULL!=m_mempool_ptr)
        m_mempool_ptr->free();
}
