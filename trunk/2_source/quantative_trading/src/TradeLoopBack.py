# -*- coding: utf-8 -*-
from  TradeStrategyBase import TradeStrategyBase;
from StockTradeDays import StockTradeDays;

class TradeLoopBack(object):
    """
    交易回测系统
    """
    def __init__(self, trade_days, trade_strategy):
        """
        使用前面封装的StockTradeDays和TradeStrategyBase类初始化交易系统
        :param trade_days:: StockTradeDays交易数据序列
        :param trade_strategy: TradeStrategyBase交易策略
        """
        self.trade_days     = trade_days;
        self.trade_strategy = trade_strategy;
        #交易盈亏结果
        self.profit_array=[];
    def execute_trade(self):
        """
        执行交回测
        :return:
        """
        for ind, day in enumerate(self.trade_days):
            """ 以时间驱动完成回测 """
            if self.trade_strategy.keep_stock_day > 0:
                #持有股票,则加入交易盈亏结果序列
                self.profit_array.append(day.change);
            #hasattr: 用来查询对象是否实现某个方法
            if hasattr(self.trade_strategy, 'buy_strategy'):
                #买入策略执行
                self.trade_strategy.buy_strategy(ind, day, self.trade_days);
            if hasattr(self.trade_strategy, 'sell_strategy'):
                #卖出策略
                self.trade_strategy.sell_strategy(ind, day, self.trade_days);
