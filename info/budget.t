.TH Command BUDGET
.NA budget "Look at a national balance sheet"
.LV Basic
.SY "budget"
The budget command predicts expenses and income for the next update.
.s1
.EX budget
.NF
Sector Type                     Production 		            Cost
park                            850 happy  		            7650
defense plant                   115 guns   		            3600
shell industry                  679 shells 		            2097
technical center                66 tech    		           27000
research lab                    7 medical  		             900
library/school                  1616 edu   		           14544
refinery                        388 petrol 		             425
enlistment center               930 mil    		            2790
Unit building                   6 units    		             358
Unit maintenance                85 units   		           28992
Ship building                   2 ships    		             480
Ship maintenance                25 ships   		            6327
Plane building                  9 planes   		            2508
Plane maintenance               219 planes 		            8874
Sector building                                                     2398
Sector maintenance		5 sectors       		     300
Military payroll                7648 mil, 338 res                  38247
Total expenses....................................................147490
Income from taxes               320709 civs, 100235 uws           +92513
Income from bars                7986 bars                         +95832
Total income.....................................................+188345
Balance forward                                                    41617
Estimated delta                                                   +40855
Estimated new treasury.............................................82472
.FI
.s1
The estimated delta may not be correct, due to events during updates:
starvation, plague, sector revolts, etc.
.s1
Budget takes into account avail and materials in headquarters,
harbors and airports, and will
only show you how much you'll pay for work that will actually get done
in cases where your headquarters/airports/harbors will not have enough avail
or materials to work on all units/planes/ships.
.s1
.L "Going broke"
.s1
The first thing you need to know about going broke: don't!  Stuff you
can't afford will not be made.  Lack of maintenance will damage your
assets.  Worst of all, because many commands are not available while
you're broke, you'll be helpless until you become solvent again
.s1
There is no easy way to predict what exactly will still be paid for
when you go broke during an update.  Try info \*QUpdate-sequence\*U.
.s1
.L "Cutting cost"
.s1
The orthodox way to balance your budget is to sack a neighbor.  In
case that isn't possible right now, a few tips:
.s1
Stop expensive sectors, ships, planes and land units.  See the
\*Qstart\*U and \*Qstop\*U commands for how.
.s1
Instead of stopping a sector, you can throttle it by moving out some
of the raw materials required for production.  The \*Qproduction\*U
command can help you fine tune.
.s1
Trim your military.  Consider demobilizing.
.s1
.SA "Sector-types, Maintenance, Nations, Updates, Update-sequence, demobilize, production, start, stop"
