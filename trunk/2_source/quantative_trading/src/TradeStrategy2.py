# -*- coding: utf-8 -*-
from TradeStrategyBase import TradeStrategyBase;
from TradeLoopBack import TradeLoopBack;
from StockTradeDays import StockTradeDays;

import itertools

class TradeStrategy2(TradeStrategyBase):
    """
    交易策略2: 均值回复策略,当股价连续两个交易日下跌 且下跌幅度超过阈值s_buy_change_threshold(10%)
              买入票并持有s_keep_stock_threadhold(默认20天)
    """
    s_keep_stock_threshold=10; #默认持有20天
    s_buy_change_threshold = -0.10; #下跌买入阈值
    def __init__(self):
        self.keep_stock_day = 0; #已持有天数

    def buy_strategy(self, trade_ind, trade_day, trade_days):
       if self.keep_stock_day==0 and trade_ind>=1:
           """
           没有持有股票 && trade_ind>=1不是交易开始第一天, 因为需要yesterday数据
           """
           today_down = trade_day.change<0; #今日股价是否下跌
           yesterday_down = trade_days[trade_ind-1].change<0; #昨日股价是否下跌
           down_rate = trade_day.change + trade_days[trade_ind-1].change;
           if today_down and yesterday_down and down_rate<TradeStrategy2.s_buy_change_threshold:
               self.keep_stock_day = 1;
       elif self.keep_stock_day > 0:
           self.keep_stock_day += 1;

    def sell_strategy(self, trade_ind, trade_day, trade_days):
        if self.keep_stock_day >= TradeStrategy2.s_keep_stock_threshold:
            """ 持有股票天数超过阈值s_keep_stock_threshold,则卖出股票 """
            self.keep_stock_day = 0;

    @classmethod
    def set_keep_stock_threshold(cls, keep_stock_threshold):
        cls.s_keep_stock_threshold = keep_stock_threshold;
    @staticmethod
    def set_buy_change_threshold(self, buy_change_threshold):
        TradeStrategy2.s_buy_change_threshold = buy_change_threshold;

def calc(keep_stock_threshold, buy_change_threshold):
    """

    :param keep_stock_threshold: 持股天数
    :param buy_change_threshold: 下跌买入阈值
    :return:盈亏情况,持股天数,下跌买入阈值
    """
    trade_strategy2 = TradeStrategy2();
    TradeStrategy2.set_keep_stock_threshold(keep_stock_threshold);
    TradeStrategy2.set_buy_change_threshold(buy_change_threshold);
    #进行回测
    trade_loop_back = TradeLoopBack(trade_days, trade_strategy2);
    trade_loop_back.execute_trade();
    #计算最终盈亏结果
    profit = 0.0 if len(trade_loop_back.profit_array)==0.0 else reduce(lambda a,b: a+b,trade_loop_back.profit_array);
    return profit,keep_stock_threshold,buy_change_threshold;
def unit_test2():
    keep_stock_list=range(2,30,2);
    print '持股天数参数组:{}'.format(keep_stock_list);
    buy_change_list=[buy_change/1000 for buy_change in range(-5,-6, -1)];
    print '下跌阈值参数组:{}'.format(buy_change_list);

    #求出最优组合
    result = [];
    for keep_stock_threshold,buy_change_threshold in itertools.product(keep_stock_list, buy_change_list):
        result.append(calc(keep_stock_threshold, buy_change_threshold))
    print '笛卡尔积参数集合总共结果:{}个'.format(len(result));
    sorted(result)[::-1][:10]

def unittest_ProcessPoolExecutor():
    """
    使用多进程(ProcessPoolExecutor)
    :return:
    """
    keep_stock_list = range(2, 30, 2);
    print '持股天数参数组:{}'.format(keep_stock_list);
    buy_change_list = [buy_change / 1000 for buy_change in range(-5, -6, -1)];
    print   '下跌阈值参数组:{}'.format(buy_change_list);

    result = [];
    def when_done(r):
        result.append(r.result());
    """
    with class_a() as a: 上下文管理器
    """
    with ProcessPoolExector() as pool:
        for keep_stock_threshold, buy_change_threshold in itertools.product(keep_stock_list, buy_change_list):
            """
            sumbit提交任务:使用calc函数和参数通过submit提交到独立的进程;
            提交的任务函数必须是简单函数,进程冰雪不支持类方法、闭包等,
            函数参数和返回值必须兼容pickle序列化,因为进程间的通信需要传递可序列化对象
            """
            future_result = pool.sumbit(calc, keep_stock_threshold, buy_change_threshold);
            #进程完成任务即calc运行结束后的回调函数
            future_result.add_done_callback(when_done);

def unittest_ThreadPoolExecutor():
    """
    使用多线程(ThreadPoolExecutor)
    :return:
    """
    keep_stock_list = range(2, 30, 2);
    print '持股天数参数组:{}'.format(keep_stock_list);
    buy_change_list = [buy_change / 1000 for buy_change in range(-5, -6, -1)];
    print   '下跌阈值参数组:{}'.format(buy_change_list);

    result = [];
    def when_done(r):
        result.append(r.result());
    """
    with class_a() as a: 上下文管理器
    """
    with ThreadPoolExector() as pool:
        for keep_stock_threshold, buy_change_threshold in itertools.product(keep_stock_list, buy_change_list):
            """
            sumbit提交任务:使用calc函数和参数通过submit提交到独立的进程;
            提交的任务函数必须是简单函数,进程冰雪不支持类方法、闭包等,
            函数参数和返回值必须兼容pickle序列化,因为进程间的通信需要传递可序列化对象
            """
            future_result = pool.sumbit(calc, keep_stock_threshold, buy_change_threshold);
            #进程完成任务即calc运行结束后的回调函数
            future_result.add_done_callback(when_done);

if "__main__" == __name__:
    trade_strategy2 = TradeStrategy2();
    TradeStrategy2.set_keep_stock_threshold(20); #修改股票持仓天数
    TradeStrategy2.set_buy_change_threshold(-.0.8); #设置股价下跌买入阈值

    trade_days = StockTradeDays([27.12, 27.03, 26.94, 26.87, 27.03, 27.45, 27.53], 20180506);
    trade_loop_back = TradeLoopBack(trade_days, trade_strategy2);
    trade_loop_back.execute_trade();
    print '回测策略2 总盈亏: {}%'.format(reduce(lambda a,b: a+b, trade_loop_back.profit_array)*100);
    #显示可视化结果
    plt.plot(np.array(trade_loop_back.profit_array).cumsum());