.TH Command BUY
.NA buy "Purchase commodities from the world market"
.LV Expert
.SY "buy <ITEM> <LOT#> <BID/UNIT> <DEST>"
The buy command allows nations to purchase commodities
on the open market.
Not all commodities can be traded -- notably civilians and military.
.s1
It is based on a bidding system, and after the bidding time has passed,
the transactions are executed, and the highest bidder gets the goods
(maybe.)  If there are no bidders once the time passes, the goods stay
on the market and the first bidder will get them.
.s1
Typical output looks like:
.EX buy food
.NF

     Empire Market Report
   Wed Mar 13 14:26:26 1996
 lot  high bid  by  time left  owner  item  amount  sector
 ---  --------  --  ---------  -----  ----  ------  ------
   0  $  10.00   1   0.00 hrs  (  1)   f        10  
   1  $   1.00   1   0.00 hrs  (  1)   f        10  

Looks just like Christmas at K-mart, doesn't it!

Which lot are you bidding on: 1
How much per unit: 2
destination sector : 3,1
Your bid is being considered.
.FI
.s1
In addition, the \*Qdestination sector\*U must be a warehouse or harbor,
and must be greater than 60% efficient to receive commodities.
.s1
.SA "reset, sell, market, Commerce"
