.TH Command MARKET
.NA market "Report current selling prices in the world market"
.LV Expert
.SY "market <ITEM|all>"
The market report displays the lowest price commodity of each type
currently on the market.
.s1
If you specify "all" for the item, all commodities on the market
are displayed.
.s1
If you specify a specific item, all lots of that item are displayed.
.s1
.EX market
.NF
     Empire Market Report
   Sun Dec 27 12:42:59 1998
 lot  high bid/unit  by  time left  owner  item  amount  sector
 ---  -------------  --  ---------  -----  ----  ------  ------
   1  $        0.50   1   1.50 hrs  (  1)   f       100  -1,1
   3  $        0.50   1   1.52 hrs  (  1)   o       100  -1,1

Looks just like Christmas at K-mart, doesn't it!
.FI
.EX market oil
.NF

     Empire Market Report
   Sun Dec 27 12:43:08 1998
 lot  high bid/unit  by  time left  owner  item  amount  sector
 ---  -------------  --  ---------  -----  ----  ------  ------
   2  $        1.00   1   1.52 hrs  (  1)   o       100  -1,1
   3  $        0.50   1   1.52 hrs  (  1)   o       100  -1,1
   4  $        0.50   1   1.52 hrs  (  1)   o       100  -1,1

Looks just like Christmas at K-mart, doesn't it!
.FI
.EX market all
.NF
     Empire Market Report
   Sun Dec 27 12:43:09 1998
 lot  high bid/unit  by  time left  owner  item  amount  sector
 ---  -------------  --  ---------  -----  ----  ------  ------
   0  $        1.00   1   1.50 hrs  (  1)   f       100  -1,1
   1  $        0.50   1   1.50 hrs  (  1)   f       100  -1,1
   2  $        1.00   1   1.52 hrs  (  1)   o       100  -1,1
   3  $        0.50   1   1.52 hrs  (  1)   o       100  -1,1
   4  $        0.50   1   1.52 hrs  (  1)   o       100  -1,1

Looks just like Christmas at K-mart, doesn't it!
.FI
.s1
.SA "buy, sell, reset, Market"
