.TH Command CENSUS
.NA census "Report contents of sectors"
.LV Basic
.SY "census <SECTS>"
The census command displays specific information on
some or all of the sectors you occupy.
.s1
In the syntax of the census command
<SECTS> is the area on which you wish information,
(see \*Qinfo Syntax\*U).
.s1
A typical usage might be:
.EX census 0:9,0:9 ?des=m
which would list data for mines in an area
to the south-east of the capital.
.s1
A census lists each of your sectors in the specified area headed by:
.NF
Tue Oct 10 22:24:49 1995
CENSUS                   del dst
  sect        eff prd mob uf uf old  civ  mil   uw food work avail ter fall coa
.FI
These columns represent:
.s1
.in \w'coast\0\0'u
.L sect
the x and y coordinates of the sector
.L eff
the efficiency of the sector (affects all benefits
of a designated sector except mobility units)
.L prd
Will this sector be updated? If not, you will see an 'n'.
This can be controlled with the \*Qstart\*U and \*Qstop\*U commands.
.L mob
the number of mobility units, (see \*Qmove\*U)
.L uf
Two delivery columns corresponding to:
uncompensated workers and food.
.L uf
Two \*Qdistribute\*U columns corresponding to:
uncompensated workers and food.
.L old
the old owner of an occupied enemy sector.
.L civ
the number of civilians
.L mil
the number of military troops
.L uw
the number of uncompensated workers
.L food
the amount of food
.L work
the percentage of civilians working in the sector
(based on the local sector happiness).
.L avail
the quantity of workforce available for construction
or maintenance of land units, planes, ships, or nukes.
.L ter
This is a territory for the sector that you can specify using the
\*Qterritory\*U command.
.L fall
radioactive contamination due to nuclear fallout
.L coast
Is this a coastal sector?
.in
.s1
For example:
.EX cens #0
.NF
Tue Jun 20 00:33:43 1989
CENSUS                   del dst
  sect        eff prd mob uf uf old  civ  mil   uw food work avail ter fall coa
   2,-2   f  100%       8 .. ..        0  499    0    0   0%   64        0
  -1,-1   f  100%      32 .. ..      629    0    0  970  43%   31    3   0
   1,-1   n  100%   n  31 ..         627    0    0  970  51%   30        0
   3,-1   )  100%      32 .. ..        0    0    0    0   0%    0        0
  -2,0    h  100%      32 .. ..      629    0    0  970 100%   93   50   0   1
   0,0    *  100%     114 .. ..      906  845    0  729 100%  173   50   0
   2,0    c  100%     127 .. ..      154  100  124   71 100%   49        0
   1,1    -    0%      32 .. ..        1    0    0    0   0%    0        0
8 sectors
.FI
.s1
.SA "improve, sinfrastructure, commodity, resource, level, territory, cutoff, map, start, stop, Sectors, Populace, Commodities"
