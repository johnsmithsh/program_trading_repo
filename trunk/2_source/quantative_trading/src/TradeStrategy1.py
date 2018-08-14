# -*- coding: utf-8 -*-
from TradeStrategyBase import TradeStrategyBase;
from TradeLoopBack import TradeLoopBack;
from StockTradeDays import StockTradeDays;

class TradeStrategy1(TradeStrategyBase):
    """
    交易策略1: 追涨策略,当股价上涨一个阈值，默认7%
              买入票并持有s_keep_stock_threadhold(默认20天)
    """
    s_keep_stock_threshold=20; #默认持有20天
    def __init__(self):
        self.keep_stock_day = 0; #已持有天数
        self.__buy_change_threshold = 0.07; #阈值
    def buy_strategy(self, trade_ind, trade_day, trade_days):
        if self.keep_stock_day == 0 and trade_day.change>self.__buy_change_threshold:
            #当前没有持仓 且 符合买入条件, 买入
            sef.keep_stodk_day = 1;
        elif self.keep_stock_day>0:
            #表示持有股票
            self.keep_stock_day += 1
    def sell_strategy(self, trade_ind, trade_day, trade_days):
        if self.keep_stock_day >= TradeStrategy1.s_keep_stock_threshold:
            #股票持有天数超过阈值,卖出股票
            self.keep_stock_day = 0;

    @property
    def buy_change_thredshold(self):
        return self.__buy_change_threshold;
    @buy_change_thredshold.setter
    def buy_change_threshold(self, buy_change_threshold):
        if not isinstance(buy_change_threshold, float):
            raise TypeError('buy_change_threshold must be float!');
        #上涨阈值取小数点两位小数
        self.__buy_change_threshold = round(buy_change_threshold);


if "__main__" == __name__:
    trade_days = StockTradeDays([27.12, 27.03, 26.94, 26.87, 27.03, 27.45, 27.53], 20180506);
    trade_loop_back = TradeLoopBack(trade_days, TradeStrategy1());
    trade_loop_back.execute_trade();
    print '回测策略1 总盈亏: {}%'.format(reduce(lambda a,b: a+b, trade_loop_back.profit_array)*100)