#include <stdio.h>
#include <string.h>

#include "mdb_define.h"

bool insert_contract_demo();
bool select_contract_demo();
bool update_contract_demo();

int main(int argc, char **argv)
{
    char szMsg[1024]={0};
    bool b_rc;
    char fastdb_file[]="testdb.fdb";
    
    printf("start fastdb demo...\n");
    printf("dbDatabaseOffsetBits=[%d]\n",dbDatabaseOffsetBits);
    const int mdb_init_size=dbDefaultInitDatabaseSize;//1<<20;        //内存库初始大小
    const int mdb_grow_granularity=dbDefaultExtensionQuantum;//4<<20; //内存库增长粒度
    const int mdb_index_init_size=dbDefaultExtensionQuantum;//64<10;  //内存库索引区大小
    const int thread_num=0;//查询并发线程数; 0-fastdb自动检测cpu个数;该参数大于cpu个数没有任何意义;
    const int sec_timeout=5;//用于避免打开失败,当前进程崩溃导致其他进程锁住;
    
    bool insert_demo_flag=(access(fastdb_file,0)!=0);
    //打开数据库
    //dbDatabase db(dbDatabase::dbConcurrentRead, mdb_init_size, mdb_grow_granularity, mdb_index_init_size);
    dbDatabase db(dbDatabase::dbConcurrentUpdate, mdb_init_size, mdb_grow_granularity, mdb_index_init_size,thread_num);
    if(!db.open("testdb", fastdb_file, sec_timeout))
    {
        printf("Error: fastdb open(%s) failed!\n", "testdb.fdb");
        //db.errorText(b_rc,)
        return -1;
    }
    
    //插入数据
    if(insert_demo_flag)
    {
        printf("insert demo...\n");
        insert_contract_demo();
    }
    
    //查询数据
    select_contract_demo();
    
    //更新数据
    update_contract_demo();
    
    
    db.close();
    
    return 0;
}

bool insert_contract_demo()
{
    FDB_TtFtrContract tb_ftr_contract;
    char exch_code[8]        ="A";
    char contract_code[32]   ="ag1809-C-3500";
    char vari_code[16]       ="ag";
    char vari_name[32]       ="黄金";
    char deliv_date[12]      ="1809";
    char last_trade_date[12] ="20201304";
    char expire_date[12]     ="20301123";
    char exec_date[12]       ="20180706";
    
    tb_ftr_contract.exch_code=exch_code;
    tb_ftr_contract.contract_code=contract_code;
    tb_ftr_contract.vari_code=vari_code;
    tb_ftr_contract.vari_name=vari_name;
    tb_ftr_contract.deliv_date=deliv_date;
    tb_ftr_contract.last_trade_date=last_trade_date;
    tb_ftr_contract.expire_date=expire_date;
    tb_ftr_contract.exec_date=exec_date;
    tb_ftr_contract.hands=10;
    tb_ftr_contract.price_unit=1.23;//最小变动价格
    tb_ftr_contract.min_hand=1;
    tb_ftr_contract.max_hand=100000;
    tb_ftr_contract.option_type='1';//期货期权类型
    tb_ftr_contract.cp_flag='C';
    tb_ftr_contract.strike_price=3500;
    insert(tb_ftr_contract);
    
    strcpy(exch_code,      "A");
    strcpy(contract_code,  "au2309-C-4500");
    strcpy(vari_code,      "au");
    strcpy(vari_name,      "白银");
    strcpy(deliv_date,     "2309");
    strcpy(last_trade_date,"19201304");
    strcpy(expire_date,    "19301123");
    strcpy(exec_date,      "19180706");
    tb_ftr_contract.exch_code=exch_code;
    tb_ftr_contract.contract_code=contract_code;
    tb_ftr_contract.vari_code=vari_code;
    tb_ftr_contract.vari_name=vari_name;
    tb_ftr_contract.deliv_date=deliv_date;
    tb_ftr_contract.last_trade_date=last_trade_date;
    tb_ftr_contract.expire_date=expire_date;
    tb_ftr_contract.exec_date=exec_date;
    tb_ftr_contract.hands=30;
    tb_ftr_contract.price_unit=4.56;//最小变动价格
    tb_ftr_contract.min_hand=11;
    tb_ftr_contract.max_hand=30000;
    tb_ftr_contract.option_type='1';//期货期权类型
    tb_ftr_contract.cp_flag='C';
    tb_ftr_contract.strike_price=4500;
    insert(tb_ftr_contract);
    
    
    strcpy(exch_code,      "B");
    strcpy(contract_code,  "WT1911-P-2500");
    strcpy(vari_code,      "WT");
    strcpy(vari_name,      "小麦");
    strcpy(deliv_date,     "1911");
    strcpy(last_trade_date,"20191304");
    strcpy(expire_date,    "20191123");
    strcpy(exec_date,      "20190706");
    tb_ftr_contract.exch_code=exch_code;
    tb_ftr_contract.contract_code=contract_code;
    tb_ftr_contract.vari_code=vari_code;
    tb_ftr_contract.vari_name=vari_name;
    tb_ftr_contract.deliv_date=deliv_date;
    tb_ftr_contract.last_trade_date=last_trade_date;
    tb_ftr_contract.expire_date=expire_date;
    tb_ftr_contract.exec_date=exec_date;
    tb_ftr_contract.hands=40;
    tb_ftr_contract.price_unit=19.56;//最小变动价格
    tb_ftr_contract.min_hand=12;
    tb_ftr_contract.max_hand=33000;
    tb_ftr_contract.option_type='1';//期货期权类型
    tb_ftr_contract.cp_flag='P';
    tb_ftr_contract.strike_price=2500;
    insert(tb_ftr_contract);
    
    return true;
}


bool select_contract_demo()
{
    printf("select demo...\n");
    //查询
    dbQuery q;
    dbCursor<FDB_TtFtrContract> contract_cursor;
    char tmp_exch_code[16];
    strcpy(tmp_exch_code, "B");
    q="exch_code=",tmp_exch_code;
    int rc=contract_cursor.select(q);
    printf("contract: exch_code=%s:\n", tmp_exch_code);
    if(rc>0)
    {
        do
        {
            printf("    =========================================\n");
            printf("    exch_code=%s\n",contract_cursor->exch_code);
            printf("    contract_code=%s\n",contract_cursor->contract_code);
            printf("    vari_code=%s\n",contract_cursor->vari_code);
            printf("    vari_name=%s\n",contract_cursor->vari_name);
            printf("    deliv_date=%s\n",contract_cursor->deliv_date);
            printf("    last_trade_date=%s\n",contract_cursor->last_trade_date);
            printf("    expire_date=\%sn",contract_cursor->expire_date);
            printf("    exec_date=%s\n",contract_cursor->exec_date);
            printf("    hands=%d\n",contract_cursor->hands);
            printf("    price_unit=%f\n",contract_cursor->price_unit);
            printf("    min_hand=%d\n",contract_cursor->min_hand);
            printf("    max_hand=%d\n",contract_cursor->max_hand);
            printf("    option_type=%c\n",contract_cursor->option_type);
            printf("    cp_flag=%c\n",contract_cursor->cp_flag);
            printf("    strike_price=%f\n",contract_cursor->strike_price);
        }while(contract_cursor.next());
    }
    else
    {
        printf("    no contract\n");
    }
    
    strcpy(tmp_exch_code, "A");
    q="exch_code=",tmp_exch_code;
    rc=contract_cursor.select(q);
    printf("select demo...\n");
    printf("contract: exch_code=%s:\n", tmp_exch_code);
    if(rc>0)
    {
        do
        {
            printf("    =========================================\n");
            printf("    exch_code=%s\n",contract_cursor->exch_code);
            printf("    contract_code=%s\n",contract_cursor->contract_code);
            printf("    vari_code=%s\n",contract_cursor->vari_code);
            printf("    vari_name=%s\n",contract_cursor->vari_name);
            printf("    deliv_date=%s\n",contract_cursor->deliv_date);
            printf("    last_trade_date=%s\n",contract_cursor->last_trade_date);
            printf("    expire_date=\%sn",contract_cursor->expire_date);
            printf("    exec_date=%s\n",contract_cursor->exec_date);
            printf("    hands=%d\n",contract_cursor->hands);
            printf("    price_unit=%f\n",contract_cursor->price_unit);
            printf("    min_hand=%d\n",contract_cursor->min_hand);
            printf("    max_hand=%d\n",contract_cursor->max_hand);
            printf("    option_type=%c\n",contract_cursor->option_type);
            printf("    cp_flag=%c\n",contract_cursor->cp_flag);
            printf("    strike_price=%f\n",contract_cursor->strike_price);
        }while(contract_cursor.next());
    }
    else
    {
        printf("    no contract\n");
    }
    
    strcpy(tmp_exch_code, "C");
    q="exch_code=",tmp_exch_code;
    rc=contract_cursor.select(q);
    printf("select demo...\n");
    printf("contract: exch_code=%s:\n", tmp_exch_code);
    if(rc>0)
    {
        do
        {
            printf("    =========================================\n");
            printf("    exch_code=%s\n",contract_cursor->exch_code);
            printf("    contract_code=%s\n",contract_cursor->contract_code);
            printf("    vari_code=%s\n",contract_cursor->vari_code);
            printf("    vari_name=%s\n",contract_cursor->vari_name);
            printf("    deliv_date=%s\n",contract_cursor->deliv_date);
            printf("    last_trade_date=%s\n",contract_cursor->last_trade_date);
            printf("    expire_date=%s\n",contract_cursor->expire_date);
            printf("    exec_date=%s\n",contract_cursor->exec_date);
            printf("    hands=%d\n",contract_cursor->hands);
            printf("    price_unit=%f\n",contract_cursor->price_unit);
            printf("    min_hand=%d\n",contract_cursor->min_hand);
            printf("    max_hand=%d\n",contract_cursor->max_hand);
            printf("    option_type=%c\n",contract_cursor->option_type);
            printf("    cp_flag=%c\n",contract_cursor->cp_flag);
            printf("    strike_price=%f\n",contract_cursor->strike_price);
        }while(contract_cursor.next());
    }
    else
    {
        printf("    no contract\n");
    }
    
    return true;
}


bool update_contract_demo()
{
    dbCursor<FDB_TtFtrContract> cursor_update(dbCursorForUpdate);
    char tmp_exch_code[16];
    char tmp_contract_code[32];
    
    printf("update demo...\n");
    
    strcpy(tmp_exch_code, "A");
    strcpy(tmp_contract_code, "au2309-C-4500");
    dbQuery q;
    int rc;
    q="exch_code=",tmp_exch_code,"and contract_code=",tmp_contract_code;
    rc=cursor_update.select(q);
    if(rc>0)
    {
        do
        {
            double strike_price=cursor_update->strike_price;
            strike_price += 1;
            cursor_update->strike_price=strike_price;
        }while(cursor_update.next());
    }
    else
    {
        printf("No record!\n");
    }
    
    dbCursor<FDB_TtFtrContract> cursor_read;
    rc=cursor_read.select(q);
    if(rc>0)
    {
        do
        {
            printf("   exch_code[%s] contract_code[%s], strike_price=[%f]\n", cursor_read->exch_code, cursor_read->contract_code, cursor_read->strike_price);
        }while(cursor_read.next());
    }
    
    //printf(" no variable select...\n");
    //q="exch_code=A and contract_code=au2309-C-4500";
    //rc=cursor_read.select(q);
    //if(rc>0)
    //{
    //    do
    //    {
    //        printf("   exch_code[%s] contract_code[%s], strike_price=[%f]\n", cursor_read->exch_code, cursor_read->contract_code, cursor_read->strike_price);
    //    }while(cursor_read.next());
    //}
    
    return true;
}
