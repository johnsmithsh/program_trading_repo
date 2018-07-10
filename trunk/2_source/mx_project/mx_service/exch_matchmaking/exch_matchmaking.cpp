#include "exch_matchmaking.h"

int double_cmp(double d1, double d2)
{
    if(d1-d2>PRICE_PRECISION)
	    return 1;
	else if(d1-d2<-PRICE_PRECISION)
	    return -1;
	return 0;
}

bool double_zero(double d1)
{
   return (0==double_zero(d1, 0.0)) ? 1 : 0; 
}

ExchMatchMakeing::ExchMatchMakeing()
{
    m_max_no=0;
}

int ExchMatchMakeing::get_new_price()
{
    return m_last_done_price;
}
int ExchMatchMakeing::match_entrust(ST_ENTRUST *buy_Order, ST_ENTRUST *sell_order,  double *out_done_price, unsigned int *out_done_qty)
{
    double new_price=get_new_price();
	//两个都是市价单
	if( (double_zero(buy_order->entrust_price)==0) && (double_zero(sell_order->entrust_price)==0) )
	{
	}
	else if(double_zero(buy_order->entrust_price)>0) && (double_zero(sell_order->entrust_price)>0)//两个是限价单
	{
	    //买家<卖价,无法撮合
	    if(double_cmp(buy_order->entrust_price, sell_order->entrust_price)<0)
    	    return -1;
		
		//确定成交手数...
		int buy_qty = buy_order->entrust_qty  - buy_order->done_qty;
		int sell_qty = sell_order->sell_order - sell_order->done_qty;
		*out_done_qty = buy_qty>sell_qty ? buy_qty : sell_qty;//成交手数
		
		//确定成交价格...
		if(double_cmp(new_price, buy_order->entrust_price)<0)//最新价>买价>=卖价
		    *out_done_price=buy_order->entrust_price;
		else if(double_cmp(sell_order->entrust_price, new_price)>0)//买价>=卖价>最新价
		    *out_done_price=sell_order->entrust_price;
		else   //买价>=最新价>=卖价
		    *out_done_price=new_price;
	}
	else //一个市价单+一个限价单撮合
	{
	}
	
	return 0;
}

//买委托: 价格:高->低; 时间:低->高; 即价格降序,时间升序;
int cmp_buy_entrust(ST_ENTRUST *order1, ST_ENTRUST *order2)
{
    //价格+时间
    if(order1->entrust_price-order2->entrust_price>PRICE_PRECISION)
	    return 1;
	else if(order1->entrust_price-order2->entrust_price<-PRICE_PRECISION)
        return -1;
	
	//系统号小,时间优先
	if(order1->sys_no<order2->sys_no)
	    return 1;
    else if(order1->sys_no>order2->sys_no)
	    return -1;
		
	return 0;
}

//卖委托: 价格:低->高; 时间:低->高; 即价格降序,时间升序;
int cmp_sell_entrust(ST_ENTRUST *order1, ST_ENTRUST *order2)
{
    //价格+时间
    if(order1->entrust_price-order2->entrust_price>PRICE_PRECISION)
	    return -1;
	else if(order1->entrust_price-order2->entrust_price<-PRICE_PRECISION)
        return 1;
	
	//系统号小,时间优先
	if(order1->sys_no<order2->sys_no)
	    return 1;
    else if(order1->sys_no>order2->sys_no)
	    return -1;
		
	return 0;
}

int ExchMatchMakeing::push_buy_entrust(ST_ENTRUST *FldOrder)
{
    std::list<ST_ENTRUST>::iterator buy_iter;
	//查找插入位置; 价格优先; 时间优先;
    for(buy_iter=m_buy_lst.begin(); buy_iter!=m_buy_lst.end(); buy_iter++)
	{
	    ST_ENTRUST &order_ref=*buy_iter;
	    if(cmp_buy_entrust(&order_ref, FldOrder)<0)
		{
			break;
		}
	}
	
	if(buy_iter==m_buy_lst.end())
	{
	    m_buy_lst.push_back(*FldOrder);
	}
	else
	{
	    m_buy_lst.insert(buy_iter, *FldOrder);
	}
	
	return 0;
}

int ExchMatchMakeing::push_sell_entrust(ST_ENTRUST *FldOrder)
{
    std::list<ST_ENTRUST>::iterator buy_iter;
	//查找插入位置; 价格优先; 时间优先;
    for(buy_iter=m_buy_lst.begin(); buy_iter!=m_buy_lst.end(); buy_iter++)
	{
	    ST_ENTRUST &order_ref=*buy_iter;
	    if(cmp_sell_entrust(&order_ref, FldOrder)<0)
		{
			break;
		}
	}
	
	if(buy_iter==m_buy_lst.end())
	{
	    m_buy_lst.push_back(*FldOrder);
	}
	else
	{
	    m_buy_lst.insert(buy_iter, *FldOrder);
	}
	
	return 0;
}