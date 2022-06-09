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

---------------------------------
#### *Excellent ACSIL Reference(s)* 
[Kory Gill - 'New Study Template'](https://github.com/korygill/technical-analysis)

[Frozen Tundra](https://github.com/FrozenTundraTrader/sierrachart)
