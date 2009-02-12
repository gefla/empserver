.TH Concept Food
.NA Food "How much people eat at the update"
.LV Expert
.s1
In Empire, as in the \*Qreal\*U world, everyone must eat to survive,
(see SPECIAL NOTE below for the exception).
All of these numbers below are subject to change.
Execute the \*Qversion\*U command to get some of this information.
Ask the deity for the rest.
.s1
Each civilian or military eats 0.0005 units of food per time unit,
(i.e. 1000 people eat 24 units of food per 48-ETU update).
.s1
The ability of a sector to grow food is indicated by its \*Qfertility\*U.
Fertility values range from 0 to 100.
.s1
Food can be grown in any kind of sector although agribusinesses
are able to grow more food with fewer workers.
.s1
Every sector produces a base level of food according to the following:
.in +0.2i
.s1
The amount of food that the sector can grow in one time unit is:
.s1
.ti +0.3i
fertility * 0.002
.\" ofgrate
.s1
The amount of food that can be harvested in one time unit is:
.s1
.ti +0.3i
(civilians + military / 5) * 0.002
.\" ofcrate
.s1
i.e. one civilian can harvest four times
.\" ofcrate / eatrate
the amount that he/she eats while
one military can harvest 4/5
.\" 0.2 * ofcrate / eatrate
of the amount that she/he eats.
.s1
Note that this means that although a sector with a fertility of 40
can grow enough food to support 160 people
.\" 40 * ofcrate / eatrate
at least 10 of them must be civilians.
In fact,
any combination of civilians and military
that meets the following two criteria will be self-sufficient:
.s1
.ti +0.3i
civilians + military \(<= 4 * fertility
.\" ofgrate / eatrate
(amount that grows)
.s1
.ti +0.3i
military \(<= 15 * civilians
.\" (ofcrate / eatrate - 1) * 5
(amount that gets harvested)
.in
.s1
In an agribusiness an \*(iFadditional\*(pF amount of food can be grown.
That amount is calculated by the following:
.in +0.2i
.s1
An efficiency factor (effact) takes into account the state of repair
of the agribusiness itself (sector efficiency, effic) and
the technology level of your farmers
(the national technology level, tlev):
.s1
.ti +0.3i
effact = (effic * (tlev + 10)) / (100 * (tlev + 20))
.s1
The amount of additional food that the sector produces
in one time unit is:
.s1
.ti +0.3i
(civilians + military / 5) * effact * fertility / 1000.
.s1
Thus, a 100% agribusiness with a fertility of 50
and an educational level of 10
has an efficiency factor is 2/3;
one civilian can harvest 0.033 units of food per time unit
beyond the food normally produced in that sector.
This extra amount is enough to support 33-three other
people while one military can harvest 0.0066,
enough to feed 13 other people.
.s1
The amount of food cultivated by 10 civilians
in 10 time units in a 100% agribusiness are:
.s1
.NF
       |  EDUCATION LEVEL  |
       |  0  |  10  |  100 |
 ------+-----+------+------+
 F  10 | 0.5 | 0.67 | 0.92 |
 E  ---+-----+------+------+
 R  50 | 2.5 | 3.33 | 4.58 |
 T  ---+-----+------+------+
 . 100 | 5.0 | 6.67 | 9.17 |
 ------+-----+------+------+
.FI
.s1
SPECIAL NOTE: Residents of a sanctuary need no food to survive;
thus no one ever starves in a sanctuary.
.in 0
.s1
On a ship the following applies:
.in +0.2i
.s1
The amount of food that can be produced on a fishing boat
in one time unit is:
.s1
.ti +0.3i
(civil * etus * fertil) / 10000;
.s1
However, if the NEWFISH option is in use, the limit is:
.s1
.ti +0.3i
(civil * etus * fertil) / 1000;
.s1
The amount of food that can be produced on any other ship
in one time unit is:
.s1
.ti +0.3i
0
.in
.s1
Note that if a ship doesn't have enough food, it will
attempt to grab unneeded food from any land units it is carrying.
.s1
The presence of food also affects the birthrate in Empire;
if there is no food in a sector no babies will be born.
A baby consumes 0.3 units of food in growing to adulthood.
.\" babyeat
However, babies are only allowed to eat half the food in a sector.
Thus if a sector could produce 25 new
civilians but only has 2 spare units of food,
(after the current population has eaten),
only three babies will be born and one unit of food will be left.
.s1
.SA "Innards, Populace, Updates"
