# -*- coding: utf-8 -*-
import numpy as np;
import matplotlib.pyplot as plt;
import scipy.stats as scs; #用于拟合

stock_cnt=200
view_days=504
stock_day_change = np.random.standard_normal((stock_cnt, view_days));

#均值期望
stock_mean = stock_day_change[0].mean();
#均方差
stock_std  = stock_day_change[0].std();
print('股票0 mean均值期望{0:.3f}元'.format(stock_mean));
print('股票0 std振幅 标准差{0:.3f}'.format(stock_std));

#绘制股票直方图
plt.hist(stock_day_change[0], bins=50, normed=True);
#linespace从股票0最小值->最大值生成数据
fit_linspace = np.linspace(stock_day_change[0].min(), stock_day_change[0].max());
#概率分布函数(PDF, probability desnsity function)
#由均值、方差来概述曲线，使用scipy.stats.norm.pdf生成拟合曲线
pdf = scs.norm(stock_mean, stock_std).pdf(fit_linspace);
plt.plot(fit_linspace, pdf, lw=2, c='r');
plt.show();