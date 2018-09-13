# -*- coding: utf-8 -*-
import numpy as np;
import pandas as pd;
import matplotlib.pyplot as plt;

stock_day_change = np.load("G:/git_repo/johnsmithsh/program_trading_repo/trunk/2_source/quantative_trading/src/stock_day_change.npy");
print(stock_day_change.shape)

pd.DataFrame(stock_day_change).head();
#pd.DataFrame(stock_day_change).head(5);
#pd.DataFrame(stock_day_change).head(:5);

stock_symbols = ['股票'+str(x) for x in range(stock_day_change.shape[0])]
#print(pd.DataFrame(stock_day_change, index=stock_symbols).head(2))
days = pd.date_range('2017-1-1', periods=stock_day_change.shape[1], freq='1d');
df = pd.DataFrame(stock_day_change, index=stock_symbols, columns=days);
df = df.T
#print(df.head(2))
df_20 = df.resample('21D', how='mean');
print(df_20)


df_stock0 = df["股票0"]
print(type(df_stock0))
print(df_stock0.head())

#df_stock0.cumsum().plot();

df_stock0_5 = df_stock0.cumsum().resample('5D', how='ohlc')
df_stock0_20 = df_stock0.cumsum().resample('21D', how='ohlc')
print(df_stock0_5.head());
#plt.show()

from abupy import ABuMarketDrawing
ABuMarketDrawing.plot_candle_stick(df_stock0_5.index,
                                   df_stock0_5["open"].values,
                                   df_stock0_5["high"].values,
                                   df_stock0_5["low"].values,
                                   df_stock0_5["close"].values,
                                   np.random.random(len(df_stock0_5)),
                                   None, 'stock', day_sum=False,
                                   html_bk=False, save=False);


