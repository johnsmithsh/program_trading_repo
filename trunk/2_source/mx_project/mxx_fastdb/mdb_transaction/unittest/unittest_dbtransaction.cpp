#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "gtest/gtest.h"


#include "mdb_transaction.h"

void begin_transaction(CMdbTransaction &mdb_transaction)
{
    mdb_transaction.clear_records();
}

void gen_transacion_data1(CMdbTransaction &mdb_transaction, std::stack<std::string> &th_stack, std::vector<ST_TABLE_RECORD *> *pMemInfo)
{
    char buffer[1024];
    unsigned int uid=0;
    if(NULL!=pMemInfo) pMemInfo->clear();//用于记录
    //记录委托信息
    for(int i=0; i<100; i++)
    {
      sprintf(buffer, "委托号%d|czcd|SR1801-C-30000|价格10.00|买开%d手", i, i*10);
      mdb_transaction.push_record("db_name1", "TtEntrust", DB_OPT_TYPE_INSERT, uid++, buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_back(mdb_transaction.top_record());
    }
    
    //记录合约信息
    for(int i=0; i<40; i++)
    {
      sprintf(buffer, "合约%d|czce|SR1801-C-%d|hands=10|delta_price=0.001", i, i*1000);
      mdb_transaction.push_record("db_name1", "TtFtrContract", DB_OPT_TYPE_INSERT, uid++, buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_back(mdb_transaction.top_record());
    }
    
    //记录持仓信息
    for(int i=0; i<1000; i++)
    {
      sprintf(buffer, "持仓%d|czce|SR1801-C-%d|持仓手数%d|avg_hold_price=%f|保证金=100.00|手续费=9.0|权利金=0.01", i, i*1000, i,12.33+i);
      mdb_transaction.push_record("db_name1", "TtCustHold", DB_OPT_TYPE_INSERT, uid++, buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_back(mdb_transaction.top_record());
    }
    
    return;
}

void gen_transacion_data2(CMdbTransaction &mdb_transaction, std::stack<std::string> &th_stack, std::vector<ST_TABLE_RECORD *> *pMemInfo)
{
    char buffer[1024];
    unsigned int uid=0;
    if(NULL!=pMemInfo) pMemInfo->clear();//用于记录
    //记录委托信息
    for(int i=0; i<200; i++)
    {
      sprintf(buffer, "copy2|委托号%d|czcd|SR1801-C-30000|价格10.00|买开%d手", i, i*10);
      mdb_transaction.push_record("db_name1", "TtEntrust", DB_OPT_TYPE_INSERT, uid++, buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_back(mdb_transaction.top_record());
    }
    
    //记录合约信息
    for(int i=0; i<70; i++)
    {
      sprintf(buffer, "copy2|合约%d|czce|SR1801-C-%d|hands=10|delta_price=0.001", i, i*1000);
      mdb_transaction.push_record("db_name1", "TtFtrContract", DB_OPT_TYPE_INSERT, uid++, buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_back(mdb_transaction.top_record());
    }
    
    //记录持仓信息
    for(int i=0; i<1101; i++)
    {
      sprintf(buffer, "copy2|持仓%d|czce|SR1801-C-%d|持仓手数%d|avg_hold_price=%f|保证金=100.00|手续费=9.0|权利金=0.01", i, i*1000, i,12.33+i);
      mdb_transaction.push_record("db_name1", "TtCustHold", DB_OPT_TYPE_INSERT, uid++, buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_back(mdb_transaction.top_record());
    }
    
    return;
}

void rollback_or_commit(CMdbTransaction &mdb_transaction)
{
    mdb_transaction.clear_records();
}


TEST(CMdbTransaction, CMdbTransaction)
{
    CMdbTransaction mdb_transaction;
    
    std::stack<std::string> th_stack;
    std::vector<ST_TABLE_RECORD *> mem_info;
    
    std::vector<ST_TABLE_RECORD *> *pMemInfo=NULL;
    
    std::vector<ST_TABLE_RECORD *>::reverse_iterator riter;
    //
    int i = 0;
    for(int i=0; i<100; ++i)
    {
        if(0==i)
            pMemInfo = &mem_info;
        else 
            pMemInfo = NULL;
        
        begin_transaction(mdb_transaction);
        gen_transacion_data1(mdb_transaction, th_stack, pMemInfo);
        //检查数据
        riter=mem_info.rbegin();
        
        ST_TABLE_RECORD *pRecord=NULL;//mdb_transaction.pop_record();
        
        int count = 0;
        while(NULL!=(pRecord=mdb_transaction.pop_record()))
        {
            ++count;
            std::string recort_str=th_stack.top();
            
            //printf("db_name=[%s], tb_name=[%s] uid=[%d]\n",pRecord->db_name, pRecord->tb_name,pRecord->record_uid);
            
            ASSERT_TRUE(riter!=mem_info.rend());
            
            ASSERT_TRUE(0==strcmp((char *)pRecord->data_ptr, recort_str.c_str()));
            ASSERT_EQ(pRecord->data_size, recort_str.size()+1);
            
            ST_TABLE_RECORD *p_mem=*riter;
            
            //printf("p_mem[%p]!=pRecord[%p], data_ptr=[%p] data_ptr-offsetof=[%p]\n", p_mem, pRecord, pRecord->data_ptr-offsetof(ST_TABLE_RECORD, data_ptr));
            ASSERT_EQ(p_mem->data_ptr, pRecord->data_ptr);
            ASSERT_EQ(p_mem, pRecord);//说明每次分配内存在内存池的位置是同一个
            
            ++riter;
            th_stack.pop();
            
        }
        
        //if(*riter!=NULL)
        //{
        //    pRecord=*riter;
        //    printf("rite=[%p]\n",pRecord);
        //    printf("db_name=[%s], tb_name=[%s]\n",pRecord->db_name, pRecord->tb_name);
        //    
        //    printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%d], data=[%s], size=%d\n",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,pRecord->data_ptr, pRecord->data_size);
        //}
        
        //++riter;
        //if(*riter!=NULL)
        //{
        //    pRecord=*riter;
        //    printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%u], data=[%s], size=%d",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,pRecord->data_ptr, pRecord->data_size);
        //}
        ASSERT_TRUE(riter==mem_info.rend());
        
        rollback_or_commit(mdb_transaction);
        
        printf("first: end of for[%d]\n", i);
    }
    
    
    std::stack<std::string> th_stack2;
    std::vector<ST_TABLE_RECORD *> mem_info2;
    for(int i=0; i<100; ++i)
    {
        if(0==i)
            pMemInfo = &mem_info2;
        else 
            pMemInfo = NULL;
        
        begin_transaction(mdb_transaction);
        gen_transacion_data2(mdb_transaction, th_stack2, pMemInfo);
        //检查数据
        riter=mem_info2.rbegin();
        
        ST_TABLE_RECORD *pRecord=NULL;//mdb_transaction.pop_record();
        
        int count = 0;
        while(NULL!=(pRecord=mdb_transaction.pop_record()))
        {
            ++count;
            std::string recort_str=th_stack2.top();
            
            //printf("db_name=[%s], tb_name=[%s] uid=[%d]\n",pRecord->db_name, pRecord->tb_name,pRecord->record_uid);
            
            ASSERT_TRUE(riter!=mem_info2.rend());
            
            ASSERT_TRUE(0==strcmp((char *)pRecord->data_ptr, recort_str.c_str()));
            ASSERT_EQ(pRecord->data_size, recort_str.size()+1);
            
            ST_TABLE_RECORD *p_mem=*riter;
            
            //printf("p_mem[%p]!=pRecord[%p], data_ptr=[%p] data_ptr-offsetof=[%p]\n", p_mem, pRecord, pRecord->data_ptr-offsetof(ST_TABLE_RECORD, data_ptr));
            ASSERT_EQ(p_mem->data_ptr, pRecord->data_ptr);
            ASSERT_EQ(p_mem, pRecord);//说明每次分配内存在内存池的位置是同一个
            
            ++riter;
            th_stack2.pop();
            
        }
        
        //if(*riter!=NULL)
        //{
        //    pRecord=*riter;
        //    printf("rite=[%p]\n",pRecord);
        //    printf("db_name=[%s], tb_name=[%s]\n",pRecord->db_name, pRecord->tb_name);
        //    
        //    printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%d], data=[%s], size=%d\n",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,pRecord->data_ptr, pRecord->data_size);
        //}
        
        //++riter;
        //if(*riter!=NULL)
        //{
        //    pRecord=*riter;
        //    printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%u], data=[%s], size=%d",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,pRecord->data_ptr, pRecord->data_size);
        //}
        ASSERT_TRUE(riter==mem_info2.rend());
        
        rollback_or_commit(mdb_transaction);
        
        printf("second: end of for[%d]\n", i);
    }
    
    for(int i=0; i<100; ++i)
    {
        //if(0==i)
        //    pMemInfo = &mem_info;
        //else 
        pMemInfo = NULL;
        
        begin_transaction(mdb_transaction);
        gen_transacion_data1(mdb_transaction, th_stack, pMemInfo);
        //检查数据
        riter=mem_info.rbegin();
        
        ST_TABLE_RECORD *pRecord=NULL;//mdb_transaction.pop_record();
        
        int count = 0;
        while(NULL!=(pRecord=mdb_transaction.pop_record()))
        {
            ++count;
            std::string recort_str=th_stack.top();
            
            //printf("db_name=[%s], tb_name=[%s] uid=[%d]\n",pRecord->db_name, pRecord->tb_name,pRecord->record_uid);
            
            ASSERT_TRUE(riter!=mem_info.rend());
            
            ASSERT_TRUE(0==strcmp((char *)pRecord->data_ptr, recort_str.c_str()));
            ASSERT_EQ(pRecord->data_size, recort_str.size()+1);
            
            ST_TABLE_RECORD *p_mem=*riter;
            
            //printf("p_mem[%p]!=pRecord[%p], data_ptr=[%p] data_ptr-offsetof=[%p]\n", p_mem, pRecord, pRecord->data_ptr-offsetof(ST_TABLE_RECORD, data_ptr));
            ASSERT_EQ(p_mem->data_ptr, pRecord->data_ptr);
            ASSERT_EQ(p_mem, pRecord);//说明每次分配内存在内存池的位置是同一个
            
            ++riter;
            th_stack.pop();
            
        }
        
        //if(*riter!=NULL)
        //{
        //    pRecord=*riter;
        //    printf("rite=[%p]\n",pRecord);
        //    printf("db_name=[%s], tb_name=[%s]\n",pRecord->db_name, pRecord->tb_name);
        //    
        //    printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%d], data=[%s], size=%d\n",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,pRecord->data_ptr, pRecord->data_size);
        //}
        
        //++riter;
        //if(*riter!=NULL)
        //{
        //    pRecord=*riter;
        //    printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%u], data=[%s], size=%d",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,pRecord->data_ptr, pRecord->data_size);
        //}
        ASSERT_TRUE(riter==mem_info.rend());
        
        rollback_or_commit(mdb_transaction);
        
        printf("third: end of for[%d]\n", i);
    }
    
}

int main(int argc, char **argv)
{
    
    testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
    
    return 0;
}