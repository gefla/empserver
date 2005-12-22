.TH Command BUDGET
.NA budget "Look at a national balance sheet & set priorities"
.LV Basic
.SY "budget [<sector type|P|S|L|M|N|A|C> <PRIORITY|~>]"
budget with no arguments produces:
.EX budget
.NF
Sector Type             Abbr    Production      Priority            Cost
park                    p       850 happy                           7650
defense plant           d       115 guns                            3600
shell industry          i       679 shells                          2097
technical center        t       66 tech                            27000
research lab            r       7 medical                            900
library/school          l       1616 edu                           14544
refinery                %       388 petrol                           425
enlistment center       e       930 mil                             2790
Unit building           L       6 units                              358
Unit maintenance        A       85 units                           28992
Ship building           S       2 ships                              480
Ship maintenance        M       25 ships                            6327
Plane building          P       9 planes                            2508
Plane maintenance       N       219 planes                          8874
Sector building                                                     2398
Military payroll                7648 mil, 338 res                  38247
City maintenance                5 cities                             300
Total expenses....................................................147490
Income from taxes               320709 civs, 100235 uws           +92513
Income from bars                7986 bars                         +95832
Total income.....................................................+188345
Balance forward                                                    41617
Estimated delta                                                   +40855
Estimated new treasury.............................................82472
.FI
.s1
Budget with an argument sets the priority of the given sector type
(or L for unit building, or A for unit maintenance, or 
P for plane building, or S for ship building, or M for ship
maintenance, or N for plane maintenance, or C to clear all)
.s1
Priorities are a positive number, or 0, or ~.
.s1
By default, budget lists only items which cost something. Non-producing
items will be listed if they are given a priority.
.s1
Update order:
.s1
Please see info \*QUpdate-sequence\*U for more details on the order things
happen in during the update..
.s1
.L "Turning sectors off"
.s1
Sector designations (or ship or plane building) with a 0 priority will not
produce, build efficiency, or gain avail.
.s1
If a sector of a type with 0 priority has been re-designated, the sector will
be torn down and rebuilt as usual (and may produce as well, depending on the
amount of work available, as normal)
.s1
If a sector of a type with a non-zero priority has been re-designated to a
type with a 0 priority, the sector will be torn down to 0%, and the designation
changed, but no further work will be done.
.s1
For other ways to control sector production, see the \*Qstart\*U and \*Qstop\*U
commands.
.s1
Costs for each sector type include costs for building efficiency in sectors
of that type. (This is true even if the sector will change to a different
type as a result of that work. For example, costs for libraries will include
all costs for work done in library sectors, even if one of the sectors is
being turned into an agribusiness)
.s1
Estimated Delta
.s1
The estimated delta may not be correct, due to events during updates.
(starvation, plague, sector revolts, etc).
.s1
Budget takes into account avail in headquarters,
harbors and airports, and will
only show you how much you'll pay for work that will actually get done
in cases where your headquarters/airports/harbors will not have enough avail
to work on all units/planes/ships.
.s1
If a cost is in [brackets], then that means that you will not pay it
because you will not have enough money.  If a cost is in (parenthesises),
then that means that you will not pay it because you have chosen not
to pay that cost (i.e. you have set the priority of that item to 0).
.s1
.SA "show, Sector-types, Nations, Updates"
