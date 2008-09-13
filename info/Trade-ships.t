.TH Concept "Trade ships"
.NA Trade-ships "How to use trade ships"
.LV Basic
Trade ships are special ships, with a different function than most Empire
ships. They are in the game only if the TRADESHIPS option is enabled.
.s1
Trade ships simulate internal or external trade. In the real world, countries
make goods that they can make cheaply, ship them somewhere they're expensive,
and sell them, making a profit.
.s1
In the empire version, you make a trade ship, using lots of cm's, and costing
$2500. You then sail it to a harbor somewhere and scuttle it. You then receive
money, with a varying rate of return.
.s1
The trade ship MUST be scuttled in a friendly harbor that is at least
2% efficient to make money. (In other sectors,
it sinks quietly, like most ships) You may scuttle it in either a harbor you
own, or the harbor of an ally. Pirates who capture a trade ship may also gain
money from it, just as if they were the lawful owner (although the news knows
the difference)
.s1
The return rate is based on the straight-line map distance from the harbor
where the trade ship was built to the harbor where it is scuttled.
.s1
You have to sail it at least 8 sectors to get any money at all. At less than
8 sectors, you get nothing, not even your $1750 back. Nothing, nada, zilch,
zip, nothing. Once you make a trade-ship, you're pretty much committed to
sailing it at least 8 sectors...

Currently, the return rates are:

.NF
		Distance sailed		Return rate
		0-7			NO MONEY
		8-13			2.5% * distance
		14-24			3.5% * distance
		25+			5.0% * distance
.FI
.s1
For example, if you sail the ship 10 sectors from the harbor where it was
built and scuttle it in a harbor there, you would get 125% of the money you
paid for it, or $2188. If you sailed it 15 sectors to a harbor and scuttled it,
you would get 153%, or $2678. 
.s1
Use the \*Qpayoff\*U command to see how much the trade ship would pay
were it scuttled in a harbour at its present location.
.s1
If you scuttle a trade ship in the harbor you do not own, you get a 20% bonus
(applied after the value is multiplied by the return rate above), AND the
person owning the harbor gets 10% of the cash you make as a bonus (the amount
before the +20% bonus is applied)
You still get the full amount, but he makes money as well.
(Peaceful trade enriches BOTH countries more than trading only internally).
For the devious, no, being allied with yourself does not mean you get +20% when
scuttling in your own harbors.
.s1
For example, using the #'s from above, if you sailed 15 sectors, and scuttled
in an allied harbor, you'd get $2188 * 1.2 = $2.6K, and your ally would get
$2188/10 = $218.
.s1
CASH GAINED IS ALSO MULTIPLIED BY THE EFFICIENCY OF THE TRADE SHIP.
So, if you get shot down to 21%, don't expect to get the full amount...
.s1
If you capture a trade ship, you are free to scuttle it in any harbor you can
get it into, receiving full cash calculated normally.
.s1
The cash return is multiplied by the efficiency of the ship, so less efficient
ships produce less money.
.s1
Captured trade ships may be scuttled as if they were yours (although you will
get some bad publicity in the news) Payoff is still based on the distance from
the harbor where the trade ship was built, not the sector in which it was
captured.
.s1
You can also use the "order" command to set up automated trading routes with
trade ships and have them navigate and automatically scuttle themselves when
they reach their destination.  See "info order" for more information on this,
but basically you can do things like this:
.NF
    Step 1) Make a harbor
    Step 2) Get cms distributed/delivered to your harbor each update.
    Step 3) Find a destination harbor to sail to
    Step 4) build tradeships in the harbor each update
    Step 5) load civs and food on the trade ships each update.
    Step 6) order the trade ships to sail and autoscuttle in the dest harbor.
.FI
.s1
You can then just repeat steps 4, 5, and 6 each update, and never worry
about your trade ships again.  Unless someone starts intercepting them,
that is...
.s1
.SA "payoff, order, Ship-types, scuttle, Ships"
