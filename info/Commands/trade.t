.TH Command TRADE
.NA trade "Buy planes/ships/land units"
.LV Expert
.SY "trade"
The trade report lists
all ships, planes and land units that are being sold and allows you
to buy them.
.s1
It is based on a bidding system, and after the bidding time has passed,
the transactions are executed, and the highest bidder gets the goods
(maybe.)  If there are no bidders once the time passes, the goods stay
on the market and the first bidder will get them.
.s1
Your bid must be higher than the currently high bid.
.s1
Typical output looks like:
.EX trade
.NF

     Empire Trade Report
  Wed Mar 13 14:42:45 1996
 lot high bid  by time left owner  description
 --- --------  -- --------- -----  -------------------------
   0 $   1000   2  0.06 hrs (  2)  tech 0 100% inf1  infantry 1 [f:21 ] #3
Which lot to buy: 0
Destination sector: 4,0
WARNING!  This market issues credit.  If you make more
  bids than your treasury can cover at the time of sale,
  you can potentially go into financial ruin, and see no
  gains.  You have been warned.

How much do you bid: 1100
Your bid on lot #0 is being considered.
.FI
.s1
The cargo of ships and land units are listed in the square
brackets, in this case, 4 guns and 40 shells.  The nuclear armament of
planes is listed in parentheses.
.s1
.SA "set, Market"
