.TH Command COMMODITY
.NA commodity "Report commodities in sectors"
.LV Basic
.SY "commodity <SECTS>"
The commodity report provides information
pertaining to the various commodities
in some or all of the sectors you occupy.
.s1
In the syntax of the commodity command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
A typical usage might be:
.EX commodity -6:6,-3:3
which would list data for the area extending
three sectors out from the capital in each direction.
.s1
The commodity report lists each of your sectors
with the following heading:
.NF

COMMODITIES deliver-- distribute
 sect     sgpidbolhr sgpidbolhr  sh  gun  pet iron dust  bar  oil  lcm  hcm rad
.FI
These columns represent:
.s1
.in \w'sgpidbolhr\0\0'u
.L sect
the x and y coordinates of the sector
.L sgpidbolhr
Ten delivery columns corresponding to:
shells, guns, petroleum (refined), iron, dust (gold),
bars of gold, oil, light construction materials,
heavy construction materials, and radioactive materials.
.L sgpidbolhr
Ten distribute columns corresponding to:
shells, guns, petroleum (refined), iron, dust (gold),
bars of gold, oil, light construction materials,
heavy construction materials, and radioactive materials.
.L sh
the number of shells in storage
.L gun
the number of guns in storage
.L pet
the number of units of refined petroleum
.L iron
the number of units of iron ore
.L dust
the number of units of gold dust
.L bar
the number of gold bars
.L oil
the amount of crude oil
.L lcm
the number of units of light construction materials
.L hcm
the number of units of heavy construction materials
.L rad
the number of units of radioactive materials
.in
.s1
For example:
.EX commodity -3:3,-2:2
.NF
.ps -1
COMMODITIES deliver-- distribute
 sect     sgpidbolhr sgpidbolhr  sh  gun  pet iron dust  bar  oil  lcm  hcm rad
  6,0   k .......... ...1....0.   0    0    0  100    0    0    0    0    1   0
 -1,1   j .......... .......0..   0    0    0   95    0    0    0    1    0   0
  1,1   t .......... .......1..   0    0    0    0   50    0   54  100    0   0
 -3,3   g ....g..... ..........   0    0    0    0   43    0    0    0    0   0
 -6,4   m ...j...... ..........   0    0    0   65    0    0    0    0    0   0
 -4,4   m ...u...... ...0......   0    0    0  144    0    0    0    0    0   0
 -2,4   g .......... ..........   0    0    0    0   23    0    0    0    0   0
  2,6   t .......... .......1..   0    0    0    0   38    0   53  130    0   0
  4,6   l .......... ..........   0    0    0    0    0    0    0  100    0   0
  4,8   t .......... .......1..   0    0    0    0   37    0   53   20    0   0
  6,8   h .......... ..........   0    0    0    0    0    0    0   35   15   0
11 sectors.
.ps +1
.FI
The direction of delivery is indicated by the same characters
that are used in the \*Qmove\*U, \*Qnavigate\*U,
and other commands to indicate direction.
The presence of a distribution threshold for a particular commodity
is indicated by a digit in the appropriate column
under \*Qdistribute\*U,
representing the hundreds digit of the threshold value.
.s1
.SA "census, Commodities, Sectors"
