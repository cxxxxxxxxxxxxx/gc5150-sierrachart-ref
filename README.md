## gc5150 Custom Studies 


### DOM: Auto Clear Recent Bid/Ask Volume 

This study automatically clears the 'Recent Bid/Ask Volume' for the symbol of the chart at specified session times. Option to also include 'Current Traded Bid/Ask Volume' when auto clearing. 

- Required Sierra Chart Version: 2368 (March 15, 2022) or higher

Source File: [gc5150_AutoClearRecentBidAskVolume.cpp](./gc5150_AutoClearRecentBidAskVolume.cpp) 

----------------------------

### Fair Value Gap Study (FVG) 

This study finds Fair Value Gaps for the given chart and draws based on configured parameters in the Study Settings. Main features: 
- Independent settings for Up and Down Gaps
- Extend until Gap filled
- Minimum Gap size in ticks
- Allow copying to other chart drawings
- Enable/Disable FVG Up and Down gaps
- Hide Gaps once filled
- Specify maximum lookback for bars to process
- Draw Midline

Source File: [gc5150_FVG.cpp](./gc5150_FVG.cpp) 

----------------------------

### RSI 
This study is a mod of stock RSI to include additional lines to reference. Example: 
- You might want a 70/30 set of lines
- And also an 80/20 set of lines

Source File: [gc5150_RSI.cpp](./gc5150_RSI.cpp) 

----------------------------

### Trading System Based on Alert Condition - Limit Order 
This study is a modification of the included 'Trading System Based On Alert Condition Study' that ships with Sierra. Main Features: 
- Option to use limit orders instead of market orders
- Auto cancel orders if not filled within x amount of bars

Source File: [gc5150_TradingSystemBasedOnAlertConditionLimitOrder.cpp](./gc5150_TradingSystemBasedOnAlertConditionLimitOrder.cpp)

---------------------------------
#### *Excellent ACSIL Reference(s)* 
[Kory Gill - 'New Study Template'](https://github.com/korygill/technical-analysis)

[Frozen Tundra](https://github.com/FrozenTundraTrader/sierrachart)
