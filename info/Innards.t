.TH Concept Innards
.NA Innards "Some details about the inner workings of the server"
.LV Expert
.s1
Much of this file is now out-of-date.  Correct information may be found
in the files Update-sequence and build.
.s1
This topic contains fairly nitty-gritty descriptions of
some of the underlying mechanisms in the Empire system.
It is not meant for the weak-of-heart or casual reader.
(And it's usually out of date.)
.s1
Sector Updates
.s1
Several characteristics of the Empire game are dependent on
sector updates --
mobility, efficiency, mining of ore, generation
of production units, etc.
.s1
An understanding of the calculations
involved is often helpful in planning the timing of various
actions
and although it is unlikely that this description
is strictly up to date,
it should provide a feel for the overall philosophy.
.s1
All commodities in a sector are kept as \*Qshort integers\*U.
The program cannot have \*Qnegative commodity values\*U, so the
theoretical capacity of a sector is 32767, while in practice
it is actually 9999 (for formatting reasons).
While it is possible to move more commodities than 9999 into
sectors, production of commodities will not go
above the level of 999.
So, an update that produces 450 units of iron in a sector that already
has 800 units can at most add 199 units.
.s1
Here is an approximate description of the algorithm that is
called whenever an update occurs:
.s1
Variables used are:
.in \w'workforce\0\0'u
.L etus
the # of Empire time units in this update
.L civ
civilians in the sector
.L mil
military in the sector
.L uw
uncompensated workers in the sector
.L desig
designation of the sector
.L effic
efficiency of the sector
.L miner
iron mineral content of the sector
.L gold
gold mineral content of the sector
.L t_level
technology level of the country
.L r_level
research level of the country
.L p_stage
plague stage in the sector
.L p_time
plague stage duration
.L workforce
an intermediate variable that represents work potential
.L work
the amount of work done in the sector
.in
.s1
Parameters used are (see the \*Qversion\*U command for actual values):
.in \w'uwbrate\0\0'u
.L fgrate
the food growth rate
.L fcrate
the food cultivation rate
.L eatrate
how much food people eat
.L obrate
birth rate
.L uwbrate
uncompensated worker birth rate
.L babyeat
how much babies eat growing up
.L bankint
bank interest rate in $/bar
.in
.nf
.s1
workforce = (civ*work/100 + uw + mil / 5.) / 100.
If workforce = 0 go away and don't update anything

/* increase sector efficiency */
If the weather is good enough for construction and we're not broke then
  effic becomes effic + work (if possible) and costs $1 for each
  percentage point gained.
Otherwise charge the $1 for each percentage point that would have been
  gained, (pay for workers to play cards).

/* grow some food */
Set dd = etu * sector_fertility * fgrate
    (this is the amount of food that can be grown in etu time units)
Set dtemp = work * 100. * fcrate
    (this is the amount of food that the people there can harvest)
If (dtemp < dd)     (if there aren't enough people to harvest it all)
    set dd = dtemp
Set foodtmp to the amount of food in the sector plus dd

/* feed the masses */
If desig is sanctuary then
    set dd equal to 0.
Else
    set dd equal to etu * (civ + mil) * eatrate
	(this is the amount of food the people need to eat)
If (dd > foodtmp) then some people will starve
    figure out what percentage of the population can be fed,
    and kill the rest (up to a maximum of 1/2 the populace)
    set food = 0
Otherwise
    set food = foodtmp - dd (with a maximum of 999)

/* population growth */
set q = etu * civ * obrate
	the number of births possible in other sectors
If q is bigger than food / (2 * babyeat) set q to food / (2 * babyeat)
	food available to mature this many babies into civilians
If q is bigger than 999 - civ set q to 999 - civ
	enough room for that many
Set food = food - q * babyeat
Set civ = civ + q

/* mobility */
Add etu to mobil (to a max of 127)

/* pay taxes */
Collect the taxes, pay the military
    civ_tax = (int) (.5 + (civs * eff * etu * civtaxrate / 100.));
    if (conquered) civ_tax /= 4.;
    uw_tax  = (int) (.5 + (uws  * eff * etu * uwtaxrate  / 100.));
    mil_pay = (int) (mil * etu * milpayrate);

    nat_money -= mil_pay
    nat_money += civ_tax + uw_tax

/* plague */
If p_stage = \*Qthird\*U kill off a bunch of people,
   alert the owner and the news and decrement p_time by dt.
   If p_time \*(<= 0 set p_stage = zero (plague has burned itself out).
If p_stage = \*Qsecond\*U report
   to the owner and the news and decrement p_time by dt.
   If p_time \*(<= 0 set p_stage = \*Qthird\*U
      and randomly reset p_time in the range of etu_per_update to
      (etu_per_update * 2).
If p_stage = \*Qfirst\*U decrement p_time by dt.
   If p_time \*(<= 0 set p_stage = \*Qsecond\*U
      and randomly reset p_time in the range of etu_per_update to
      (etu_per_update * 2).
If p_stage = zero and a random number in the range 0-99
   is less than the figure generated by the equation in \*Qinfo plague\*U
   then set p_stage = \*Qfirst\*U
   and set p_time to a random number in the range of etu_per_update to
   (etu_per_update * 2).
.s1
/* delivery & distribution */
If anything is being delivered from this sector and there is
more of it than the delivery threshold (always a multiple of 8)
and the country is not broke
    deliver as much of the excess as mobility allows.
    If plague_stage is \*Qsecond\*U (the infectious stage)
	set plague_stage and plague_time in the destination sector.
If there is a \*Qdistribution threshold\*U for this sector, and
if the sector isn't at this threshold,
	import or export as necessary from the distribution warehouse
	the number of commodities, mobility permitting.

/* production */
If effic is less than 60 skip the rest.
.in +3
.s1
.ti -3
If desig is bank then accrue  etu * bankint  interest per gold bar
.ti -3
If desig is capital pay etu * $1 for government salaries
.ti -3
If desig is enlistment sector, then convert civilians to military
	newmil = (etu * (mil + 10) * 0.05);
	nat_money -= newmil * 3;
.ti -3
If this sector produces something (mines, research labs, etc.)
   calculate how much can be produced (see \*Qinfo Products\*U)
       (Note that the amount that can be produced is limited by \*Qwork\*U)
   produce it
   pay for it (money, iron, gold mineral, oil, etc.)
.fi
.in
.s1
Several points are noteworthy:
.s1
\(bu The work done in a sector (ore dug up, efficiency growth,
population growth, products generated, etc) is dependent on the
product of time since last update and work force (\*Qwork\*U above)
while the accumulation of mobility is independent of work force.
.s1
\(bu If the population of a sector is very low it may never generate
any work at all due to conversion to integer truncation.
.s1
Ship Updates
.s1
Ships are also updated only when accessed however the mechanism is simpler.
The only characteristics that are changed by ship updates are
the mobility,
the efficiency, (if less than 100%).
the food, (and therefore the crew if starvation occurs),
the amount of oil,
and
plague status, (which can also change the size of the crew).
.s1
The algorithm is essentially:
.in +3
.nf
add etu to mobility (with a maximum of 127)
add etu to efficiency (with a maximum of 100)
add etu * civil * sector_oil / 10000. to oil if ship type is oil derrick
add etu * civil * sector_fertil / 10000. to food if ship type is fishing boat
feed the crew and passengers
    starve a few if not enough food
check for plague progress,
    (the same as in sectors except each stage lasts twice as long on a ship)
.fi
.in
.s1
Bureaucratic Time Units
.s1
There is one further update that is not handled in the sector
update routine; that is the update of bureaucracy time units (BTU's).
These are the numbers printed in brackets before the command
prompt.
Most commands given use BTU's, some use 1, some use 2 and some use
more, making BTU's a vital commodity.
The generation of BTU's is
dependent on the efficiency and the work force in the capital sector.
(see info BTU).
.s1
.s1
.SA "Education, Happiness, Products, Research, Technology, Time, BTU, Obsolete"
