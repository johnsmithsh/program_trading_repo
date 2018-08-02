#include <stdio.h>
#include <string.h>

#include "gtest/gtest.h"


#include "mdb_transaction.h"

void begin_transaction(CMdbTransaction &mdb_transaction)
{
    mdb_transaction.clear_records();
}

void gen_transacion_data1(CMdbTransaction &mdb_transaction, std::stack<std::string> &th_stack, std::vector<ST_TABLE_RECORD *> *pMemInfo)
{
    char buffer[1024];
    
    if(NULL!=pMemInfo) pMemInfo->clear();//用于记录
    //记录委托信息
    for(int i=0; i<100; i++)
    {
      sprintf(buffer, "委托号%d|czcd|SR1801-C-30000|价格10.00|买开%d手", i, i*10);
      mdb_transaction.push_record("db_name1", "TtEntrust", buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_front(mdb_transaction.top());
    }
    
    //记录合约信息
    for(int i=0; i<40; i++)
    {
      sprintf(buffer, "合约%d|czce|SR1801-C-%d|hands=10|delta_price=0.001", i, i*1000);
      mdb_transaction.push_record("db_name1", "TtFtrContract", buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_front(mdb_transaction.top());
    }
    
    //记录持仓信息
    for(int i=0; i<1000; i++)
    {
      sprintf(buffer, "持仓%d|czce|SR1801-C-%d|持仓手数%d|avg_hold_price=%d|保证金=100.00|手续费=9.0|权利金=0.01", i, i*1000, i,12.33+i);
      mdb_transaction.push_record("db_name1", "TtCustHold", buffer, strlen(buffer)+1);
      th_stack.push(std::string(buffer));
      if(NULL!=pMemInfo) pMemInfo->push_front(mdb_transaction.top());
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
            //printf("db_name=[%s], tb_name=[%s], opt_type=[%c] oid=[%u], data=[%s], size=%d",pRecord->db_name, pRecord->tb_name,pRecord->opt_type, pRecord->record_uid,record_uid->data_ptr, record_uid->data_size);
            
            EXPECT_TRUE(riter!=mem_info.rend());
            
            EXPECT_TRUE(0==strcmp((char *)pRecord->data_ptr, recort_str.c_str()));
            EXPECT_EQ(pRecord->data_size, recort_str.size()+1);
            
            ST_TABLE_RECORD *p_mem=*riter;
            EXPECT_EQ(p_mem, pRecord);
            
            ++riter;
            th_stack.pop();
            
        }
        
        ++riter;
        EXPECT_EQ(riter==mem_info.rend());
        rollback_or_commit(mdb_transaction);
    }
    
}

int main(int argc, char **argv)
{
    
    testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
    
    return 0;
}