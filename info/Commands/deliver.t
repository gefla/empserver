.TH Command DELIVER
.NA deliver "Establish delivery routes for shells, ore, etc"
.LV Expert
.SY "deliver <COMM> <SECTS> [q|<THRESH>] [<DIR>]"
.s1
An example of using deliver in addition to distribute is where
you may have a mine which uses distribute to get its food
from a warehouse, but uses deliver to export its iron to the adjacent
hcm factory.
.s1
Deliver may be used to extra-fine tune a distribution network.  In
fact, deliver can greatly improve the performance of a distribution
network if used properly.
.s1
If 'q' is specified rather than a threshold, then the delivery
directions and thresholds will simply be listed.  You can also use the
"cutoff" command to list delivery thresholds.
.s1
The number <THRESH> specifies how much of that commodity you would
like to be kept in the sector.  If you do not want to change the
direction that a sector delivers to, then put a '+' before the number
<THRESH>.
.s1
Note that delivery thresholds are stored as multiples of 8.  If you
specify a threshold that is not a multiple of 8, it will be rounded down.
.s1
The direction must be specified as one of the six direction characters,
(usually `y' `u' `j' `n' `b' or `g'), the \*Qno-direction\*U character,
(usually `h').  If no direction is specified, then the direction will
not be changed for that sector.
.s1
.EX "deli f 0,0 0 j"
Will deliver all food from 0,0 to 2,0.
.EX "deli i * ?des=m +87"
Will set iron delivery threshold to 80 in all mines.
.EX "deli l * q"
Will list all lcm delivery specifications.  (Sectors with no delivery
threshold or direction will not be listed).
.s1
The census and commodity reports will indicate deliveries by the 
same characters used to specify them in this command.
.s1
If, during an update, the destination sector is not owned by you,
the message \*Qdelivery walkout between x,y & x,y\*U will be printed
and nothing will be delivered.
.s1
Delivery routes may also be mapped out with the \*Qroute\*U command.
.s1
See "info Mobility" for an explanation of mobility costs.
.s1
.SA "cutoff, census, commodity, route, distribute, Distribution"
