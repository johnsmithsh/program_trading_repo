# -*- coding: utf-8 -*-
from collections import namedtuple;
from collections import OrderedDict;

class StockTradeDays(object):
    def __init__(self, price_array, start_date, date_array=None):
        """
        :param price_array: 每日价格列表
        :param start_date:  开始日期,如果data_array非空,则该参数无意义;
        :param date_array:  指定日期列表,必须与price_array对应
        """

        #价格序列
        self.__price_array = price_array;
        #日期序列
        self.__date_array = self._init_days(start_date, date_array);
        #涨跌幅度(涨幅比例)序列
        self.__change_array=self.__init_change();
        #组装字典 OrderedDict
        self.stock_dict=self._init_stock_dict();

    def __init_change(self):
        """
        根据价格序列生成涨跌幅度序列
        :return:
        """
        price_float_array=[float(price_str) for price_str in self.__price_array];
        #价格序列形成(前一个价格，后一个价格)的序列
        pp_array=[(price1, price2) for price1,price2 in zip(price_float_array[:-1], price_float_array[1:])];
        #计算涨跌幅比例
        change_array=map(lambda pp:reduce(lambda a,b:round((b-a)/a,3),pp), pp_array);
        change_array.insert(0,0);#第一天涨幅设置为0
        return change_array;
    def _init_days(self, start_date,date_array):
        """
        :param start_date:初始日期
        :param date_array:给定日期序列
        :return:
           返回交易日期列表,字符串格式列表
        """
        if date_array is None:
            #由start_date和price_array确定日期
            date_array = [str(start_date+ind) for ind,_ in enumerate(self.__price_array)];
        else:
            date_array = [str(date) for date in date_array];
        return date_array;

    def _init_stock_dict(self):
        """
        生成每日交易数据, orderedDict, key=交易日期; value-
        :return:
        """
        stock_nametuple=namedtuple('stock', ('date', 'price', 'change'));#定义
        stock_dict=OrderedDict((date, stock_nametuple(date, price, change))
                               for date, price, change in zip(self.__date_array, self.__price_array, self.__change_array)
                               );
        return stock_dict;

    def filter_stock(self, want_up=True, want_calc_sum=False):
        """
        :param want_up: 筛选上涨标记
        :param want_calc_sum:计算涨跌幅度和标记
        :return:
        """
        #三目表达式
        if not isinstance(stock_array_dict, OrderedDict):
            raise TypeError("stock_array_dict must be OrderedDict!");

        #python 三目运算符 条件为真的结果 if 条件 else 条件为假的结果
        filter_func=(lambda day: day.change >0) if want_up else (lambda day: day.change<0);
        want_days = filter(filter_func, self.stock_dict.values());

        #不需要计算汇总
        if not want_calc_sum:
            return want_days;

        #需要计算涨跌幅度,不知道有没有意义,这是涨跌百分比, 不是每日涨跌价格幅度
        change_sum=0.0
        for day in want_days:
            change_sum += day.change;

        return change_sum;

    def __str__(self):
        return str(self.stock_dict);
    def __iter__(self):
        for key in self.stock_dict:
            yield self.stock_dict[key]
    def __getitem__(self, ind):
        date_key=self.__date_array[ind];
        return self.stock_dict[date_key];
    def __len__(self):
        return len(self.stock_dict);

def unit_test():
    #单元测试: 生成函数
    ma000002_trade_days=StockTradeDays([27.12,27.03,26.94,26.87, 27.03,27.45,27.53], 20180506);
    print(ma000002_trade_days);   #__str__()
    print("len=%s" % len(ma000002_trade_days)); #__len__()
    print(ma000002_trade_days[2]); #__getitem__()

    #迭代器__iter__
    from collections import Iterable
    if isinstance(ma000002_trade_days, Iterable):
        for day in ma000002_trade_days:
            print(day);


if "__main__" == __name__:
    unit_test();