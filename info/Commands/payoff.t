.TH Command PAYOFF
.NA payoff "Find out the current payoff of a trade ship"
.LV Expert
.SY "payoff [<SHIP/FLEET> | <SECTS>]"
The payoff command gives a census of your trade ships, and lists the
payoff they would give if scuttled at the current sector.  Payoff can
only be calculated for trade ships you built yourself, not for ships
you `acquired' from somebody else.
.s1
The <SHIP/FLEET> and <SECTS> arguments are provided in case you only
wish to look at one ship
or one fleet or all ships within a given area.
.s1
Payoff expects an argument.  To see every trade ship you own, give it
the argument '*', or type "ship *".
.s1
The report format contains the following fields:
.s1
.in \w'shiptype\0\0'u
.L shp#
the ship number
.L shiptype
the type of ship; \*Qpatrol boat\*U, \*Qsubmarine\*U, etc,
.L x,y
the ship's current location (relative to your capital),
.L $$
the money you would get if you scuttled this ship
.s1
For example:
.EX payoff *
.NF
shp#     ship type  orig x,y       x,y    dist $$
   5 ts2 trade ship 2    2,2      21,-13    17 $5050.00
1 ship
.FI
.s1
Note that this command is only enabled if the TRADESHIPS option is in use.
.s1
.SA "Trade-ships, sonar, torpedo, upgrade, Ships"
