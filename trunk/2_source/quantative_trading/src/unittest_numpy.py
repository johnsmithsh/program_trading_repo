# -*- coding: utf-8 -*-
import numpy as np;
import matplotlib.pyplot as plt;

a_investor = np.random.normal(loc=100, scale=50,size=(100,1));
b_investor = np.random.normal(loc=100, scale=20,size=(100,1));

print('a交易者期望{0:.2f}元, 标准差{1:.2f}, 方差{2:.2f}'.format(a_investor.mean(), a_investor.std(), a_investor.var()));
print('b交易者期望{0:.2f}元, 标准差{1:.2f}, 方差{2:.2f}'.format(b_investor.mean(), b_investor.std(), b_investor.var()));

a_mean = a_investor.mean();
a_std  = a_investor.std();
plt.plot(a_investor);
plt.axhline(a_mean + a_std, color='r');
plt.axhline(a_mean, color='y');
plt.axhline(a_mean - a_std, color='g');
plt.show();

b_mean = b_investor.mean();
b_std  = b_investor.std();
plt.plot(b_investor);
plt.axhline(a_mean + a_std, color='r');
plt.axhline(a_mean, color='y');
plt.axhline(a_mean - a_std, color='g');
plt.show()