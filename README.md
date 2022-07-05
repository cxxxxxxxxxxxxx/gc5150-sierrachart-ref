## gc5150 Custom Studies 
### DISCLAIMER 

##### The gc5150 custom studies are provided free of charge and “as is” without warranty of any kind, either expressed or implied and are to be used at your own risk. They may contain errors and/or issues that I'm unaware of. While I do use some of these myself you should first thoroughly back test in simulation mode before introducing any new studies into your trading environment. The source code is provided so you may examine and determine for yourself if it meets your needs. By downloading and installing you accept all liability for any positive or negative results that occur while using any of these studies. Thank you and I hope you find them useful. The goal here is learning, sharing and always improving. I am by no means an expert in Sierra ACSIL Programming. I'm doing this for learning purposes and helping others. I may not have the most optimal design/code etc... I will continually improve these studies where I can.
----------------------------

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

### Forex Factory

This study downloads the weeks Forex Events from [Forex Factory](https://www.forexfactory.com) and displays them on your chart. Main features: 
- Filter by all currencies (Countries)
- Filter by Expected Impact
- Alerts via Subgraphs to color candles 10m before event
- Alerts via Subgraphs to color candles at time of event (alert active for 1 minute)
- Hide previous events
- Show tomorrow's events
- Adjust color settings independently for past/upcoming/current events
- Uses FF default colors for Event Impact markers
- Adjust font size
- Adjust horizontal and vertical offset
- Adjust event spacing
- Note: Not a feature, but does NOT currently work with TPO charts

Source File: [gc5150_ForexFactory.cpp](./gc5150_ForexFactory.cpp)  
Requires: [nlohmann json.hpp include](./nlohmann/json.hpp)  
Pre-Compiled DLL: [Compiled with SC 2408](./bin/gc5150_ForexFactory_64.dll)  

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

----------------------------

### No Mo Paper Hands
This is a 'for fun' study that has issues of it's own. The idea is if you continually close trades too early
then maybe you need a reminder not to... so you can load this study and it will continually lock trading when
you have active positions. But the 'issues of it's own' is that you can always be extra super paper hand and just
remove the study and then unlock trading again to bail :) One issue I ran into is with non Teton order routing.
Basically client side vs server side. I believe I fixed the client side issues by just cancel/flatten all orders
if you are stopped or profit target hit. Otherwise you have issues as discussed in this thread: https://www.sierrachart.com/SupportBoard.php?ThreadID=73330

Source File: [gc5150_NoMoPaperHands.cpp](./gc5150_NoMoPaperHands.cpp) 

----------------------------

### Rango Pinch
This is a Pinch study based on MACD/ADX pinch-release. To visually see pinches and releases you can toss
a MACD and ADX study on your chart and take a look. Try it on 512t chart and 1m chart. Some people will put
it on multiple time frame/tick/volume charts and look at the confluence between the pinches forming and releasing.
Written mainly as an exercise in learning Sierra studies, named after a Bearded Dragon.

Source File: [gc5150_RangoPinch.cpp](./gc5150_RangoPinch.cpp) 

----------------------------

### Output Bar Formula Value To Subgraph
This study will take the result of the given formula and place the output in a subgraph. This was written as a helper study to avoid
using Spreadsheets to perform the same function. We couldn't find an alerternative method to accomplish the same thing. So gave this a shot.

Source File: [gc5150_OutputBarFormulaValueToSubgraph.cpp](./gc5150_OutputBarFormulaValueToSubgraph.cpp) 

----------------------------

---------------------------------
#### *Excellent ACSIL Reference(s)* 
[Kory Gill - 'New Study Template'](https://github.com/korygill/technical-analysis)

[Frozen Tundra - Sierra Github Repo](https://github.com/FrozenTundraTrader/sierrachart)

[Scott Edwards - Building Sierra Chart Custom Studies](https://scottedwards.tech/post/building-sierra-chart-custom-studies)
