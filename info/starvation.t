.TH Command STARVATION
.NA starvation "Find out who's going to starve"
.LV Basic
.SY "starvation [<SECTS>|l <UNITS>|s <SHIPS>]"
The starvation command warns you of possible starvation in your
sectors, land units, and ships.
.SY starv
With no arguments, the starvation command will list all of your
sectors, land units, and ships that will starve at the update.
.SY "starv <SECTS>"
.s1
With this syntax, the starvation command displays all sectors in
<SECTS> that currently do not have enough food. It does not take into account
emergency food growing & supplying (see info supply)
It also does not worry about feeding babies... it merely
check for starvation. If you want population increase,
you'll need to chuck in some extra food...
.s1
For example:
.EX starv #0
.NF
Mon Aug 28 04:53:38 1995
Starvation
  sect         eff
   4,0    +    100% will starve 125 people. 15 more food needed
1 sector
.FI
.s1
.SY "starv l <UNITS>"
With this syntax, the starvation command will list which of your units
are currently not carrying enough food (see "info lload" and "info
supply").  For example:
.EX starv l *
.NF
Mon Aug 28 04:53:05 1995
Starvation
 lnd#     unit type
    6 cavalry           will starve 9 mil. 1 more food needed
    7 cavalry           will starve 13 mil. 2 more food needed
2 units
.FI
.s1
.SY "starv s <SHIPS>"
With this syntax, the starvation command will list which of your ships
are currently not carrying enough food to feed the people on them.
For example:
.EX starv s * ?type=frigate
.NF
Mon Aug 28 04:53:29 1995
Starvation
 shp#     ship type
    0 frigate           will starve 10 people. 2 more food needed
1 ship
.FI
.s1
To see how much food your people need to eat, check the output
of the version command.
.s1
.SA "census, version, Populace, Updates"
