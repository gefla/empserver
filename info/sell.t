.TH Command SELL
.NA sell "Sell commodities on the world market"
.LV Expert
.SY "sell <COMMODITY> <SECT> <AMOUNT> <PRICE>"
The sell command allows nations to sell commodities on the open market.
Not all commodities can be traded --
notably civilians and military.
.s1
The <COMMODITY> is the type of commodity you are selling,
<SECT> represents the sector which will be selling items.
Only harbors and warehouses
which are at least 60% efficient may sell goods.
.s1
<AMOUNT>, if positive,
reflects the amount of food to be taken from each sector.
If negative,
the amount shows the \*Qlower limit\*U of food required to be
present in that sector and will only sell food in excess of that limit.
<PRICE> is the base price of the commodity per unit to the world.
.s1
In addition, the sector selling needs to have at least 1 mobility to
sell goods.
.s1
Typical output looks like:
.NF
[7:640] Command : sell food 4,-2 10 5
Sold 10 food at 4,-2 (206 left)

[7:639] Command : sell food 4,-2 -100 5
Sold 106 food at 4,-2 (100 left)
.FI
.s1
Only harbors and warehouses
may sell items.  Also, note that due to the
time delay market, you don't get any money until the goods are actually
bidded upon and paid for once the bidding time is up.
.s1
There is a certain overhead on selling items, based on the difficulty
of moving the items in question.  This is based on the tax you pay
which is set by the deity (see \*Qinfo version\*U).
.s1
.SA "buy, reset, Commerce"
