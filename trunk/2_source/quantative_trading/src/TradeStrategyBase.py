# -*- coding: utf-8 -*- 
import six;
from abc import ABCMeta, abstractmethod;

class TradeStrategyBase(six.with_metaclass(ABCMeta, object)):
  @abstractmethod
  def buy_strategy(self, *args, **kwargs):
    #买入基类
    pass
  @abstractmethod
  def sell_strategy(self, *args, **kwargs):
    #卖出基类
    pass
