#ifndef __MXX_EXCH_MATCHMAKING_
#define __MXX_EXCH_MATCHMAKING_

#define PRICE_PRECISION 0.000001 //!< 价格精度

#define BUY_DIRECTION  '0'
#define SELL_DIRECTION '1'

//委托单
typedef struct
{
    unsigned int sys_no;     //!<委托单交易所id, 即系统号
	unsigned int entrust_qty;//!<委托手数
	unsigned int done_qty;   //!<成交手数
	char bs_flag;//买卖标记
	//char eo_flag;//
	char done_attr;//!<成交属性
	double entrust_price; //!< 委托价格
	double done_avg_price;//!< 成交均价	
}ST_ENTRUST;

//撮合成交单
typedef struct
{
    unsigned int b_sys_no;//!< 买方系统号
	unsigned int s_sys_no;//!< 卖方系统号
	unsigned int done_qty;//!< 成交手数
	double done_price;//!< 成交价
}ST_MATCH_DONE;

//交易所撮合
class ExchMatchMaking
{
  public:
    ExchMatchMaking();
  public:
    //
	int match();
    int match_entrust(ST_ENTRUST *FldOrder, std::list<ST_MATCH_DONE> *entrust_done_list);
    int push_entrust(ST_ENTRUST *FldOrder);
	
	
	
  private:
    std::list<ST_ENTRUST> m_buy_lst;
    std::list<ST_ENTRUST> m_sell_lst;
	
	std::list<ST_MATCH_DONE> m_done_lst;
	unsigned int m_max_no;
	
	double m_last_done_price;
};
#endif
