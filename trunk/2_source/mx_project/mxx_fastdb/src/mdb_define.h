#ifndef __MDB_DEFINE_H_
#define __MDB_DEFINE_H_

#include "fastdb.h"
//客户信息表
class FDB_TtCustInfo
{
  public:
    char const *cust_no;
    char const *cust_name;
    int1       cust_type;//客户类型; uchar1
    int1       gender;//性别; uchar
    int1       cust_status;//客户状态;
    char const *open_date;//开户日期
    char const *phone;
    char const *mobile;
    char const *email;
    char const *addr;
    char const *oper_date;
    char const *oper_time;
  public:
    TYPE_DESCRIPTOR((
                     FIELD(cust_no),
                     FIELD(cust_name),
                     FIELD(cust_type),
                     FIELD(gender),
                     FIELD(cust_status),
                     FIELD(open_date),
                     FIELD(phone),
                     FIELD(mobile),
                     FIELD(email),
                     FIELD(addr),
                     FIELD(oper_date),
                     FIELD(oper_time)
                   ));
};

//合约表
class FDB_TtFtrContract
{
  public:
    char const *exch_code;
    char const *contract_code;
    char const *vari_code;
    char const *vari_name;
    char const *deliv_date;
    int4       hands;
    real8      price_unit;//最小变动价格
    int4       min_hand;
    int4       max_hand;
    int1       option_type;//期货期权类型
    int1       cp_flag;
    real8      strike_price;
    char const *last_trade_date;
    char const *expire_date;
    char const *exec_date;
  public:
    TYPE_DESCRIPTOR((
                     FIELD(exch_code),
                     FIELD(contract_code),
                     FIELD(vari_code),
                     FIELD(vari_name),
                     FIELD(deliv_date),
                     FIELD(hands),
                     FIELD(price_unit),
                     FIELD(min_hand),
                     FIELD(max_hand),
                     FIELD(option_type),
                     FIELD(cp_flag),
                     FIELD(strike_price),
                     FIELD(last_trade_date),
                     FIELD(expire_date),
                     FIELD(exec_date)
                   ));
};

//REGISTER(FDB_TtCustInfo);
//REGISTER(FDB_TtFtrContract);
#endif
