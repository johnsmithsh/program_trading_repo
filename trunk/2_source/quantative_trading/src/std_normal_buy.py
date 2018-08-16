# -*- coding: utf-8 -*-
import numpy as np;
import matplotlib.pyplot as plt;
import scipy.stats as scs; #用于拟合



#可视化
def show_buy_lower(stock_ind):
    """
    可视化股票前454走势图及从第454买入后的走势图
    :param stock_ind: 股票序号
    :return: 最终盈亏
    """
    #设置一行两列的可视化图标
    _,axs = plt.subplots(nrows=1, ncols=2, figsize=(16,5));
    #绘制前454天走势图,np.cumsum()序列连续求和
    axs[0].plot(np.arange(0, view_days-keep_days),
                stock_day_change_test[stock_ind].cumsum());
    #从第454天开始到底504的走势图
    cs_buy = stock_day_change[stock_ind][view_days-keep_days:view_days].cumsum();
    axs[1].plot(np.arange(view_days-keep_days,view_days), cs_buy)

    #plt.show();
    return cs_buy[-1];

stock_cnt=200
view_days=504
stock_day_change = np.random.standard_normal((stock_cnt, view_days));

keep_days=50;
stock_day_change_test=stock_day_change[:stock_cnt, 0:view_days-keep_days];

#打印前454天数找过你股票跌幅最大的3个股票
print (np.sort(np.sum(stock_day_change_test, axis=1))[:3]);
#获取股票序号
stock_lower_array=np.argsort(np.sum(stock_day_change_test, axis=1))[:3]
#显示序号
print(stock_lower_array)

show_buy_lower(stock_lower_array[0]);
show_buy_lower(stock_lower_array[1]);
show_buy_lower(stock_lower_array[2]);

plt.show();