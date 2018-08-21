# -*- coding: utf-8 -*-
import numpy as np;
import matplotlib.pyplot as plt;
import scipy.stats as scs; #用于拟合

#赌徒个数
gamblers = 100
def casino(win_rate, win_once=1, loss_once=1, commission=0.01):
    """
    赌场: 简单设定每个赌徒1000000元,并且每个赌徒想玩10000000次
    :param win_rate: 输赢概率
    :param win_once: 每次赢的钱数
    :param loss_once: 每次输的钱数
    :param commission: 手续费
    :return:
    """
    my_money = 1000000
    play_cnt = 10000000
    commission = commission;
    for _ in np.arange(0, play_cnt):
        #使用伯努利分布,根据win_rate获取输赢
        w = np.random.binomial(1, win_rate);
        if w: #赢了
            my_money += win_once;
        else:
            my_money -= loss_once;
        #手续费
        my_money -= commission;
        if my_money<=0: #没钱了,不能玩
            break;
    return my_money;

#胜率0.5,赔率1,无手续费
heaven_moneys = [ casino(0.5, commission=0) for _ in np.arange(0, gamblers)]

#胜率0.4,赔率1,没有手续费
cheat_moneys = [ casino(0.4, commission=0) for _ in np.arange(0, gamblers)]

#胜率0.4,赔率1,没有手续费
commission_moneys = [ casino(0.5, commission=0.01) for _ in np.arange(0, gamblers)]

plt.hist(heaven_moneys, bins=30);
plt.show();