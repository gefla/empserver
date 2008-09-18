.TH Command CUTOFF
.NA cutoff "List delivery thresholds"
.LV Expert
.SY "cutoff <SECTS>"
The cutoff report provides information about the various
delivery cutoff levels in some or all of the sectors that
you occupy.
.s1
In the syntax of the level command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
A typical usage might be:
.EX cutoff -6:6,-3:3
which would list data for the area extending three sectors
out from the capital in each direction.
.s1
The cutoff report lists each of your sectors with the following heading:
.NF
DELIVERY CUTOFF LEVELS
  sect    ufsgpidbolhr   uw food  sh gun pet iron dust bar oil lcm hcm rad
.FI
These columns represent:
.s1
.in \w'ufsgpidbolhr\0\0'u
.L sect
the x and y coordinates of the sector
.L ufsgpidbolhr
the fourteen delivery direction columns correspond to the delivery
direction for civilians, military, food, guns, petroleum (refined),
iron ore, dust (gold), bars (dust), oil, lcm, and hcm.
.L civ
the delivery cutoff threshold for civilians in that sector
.L mil
the delivery cutoff threshold for military in that sector
.L uw
the delivery cutoff threshold for uncompensated workers in that sector
.L food
the delivery cutoff threshold for food in that sector
.L gun
the delivery cutoff threshold for guns in that sector
.L sh
the delivery cutoff threshold for shells in that sector
.L pet
the delivery cutoff threshold for petroleum in that sector
.L iron
the delivery cutoff threshold for iron ore in that sector
.L dust
the delivery cutoff threshold for gold dust in that sector
.L bar
the delivery cutoff threshold for gold bars in that sector
.L oil
the delivery cutoff threshold for oil in that sector
.L lcm
the delivery cutoff threshold for light construction materials
in that sector
.L hcm
the delivery cutoff threshold for heavy construction materials
in that sector
.L rad
the delivery cutoff threshold for radioactive materials in that sector
.in
.s1
For example:
.EX cutoff -3:3,-2:0
.NF
.ps -1
Sat May 31 18:40:57 2008
DELIVERY CUTOFF LEVELS
   sect   cmufsgpidbolhr civ mil  uw food sh gun pet irn dst bar oil lcm hcm rad
  0,0   c ..............   0   0   0   0   0   0   0   0   0   0   0   0   0   0
 -5,-3  a ..n...........   0   0   0 400   0   0   0   0   0   0   0   0   0   0
 -6,-2  k ..j...........   0   0   0 400   0   0   0   0   0   0   0   0   0   0
 -4,-2  + ..j...........   0   0   0 400   0   0   0   0   0   0   0   0   0   0
 -2,-2  m ......n.......   0   0   0   0   0   0   0   0   0   0   0   0   0   0
 -5,-1  t ..............   0   0   0   0   0   0   0   0   0   0   0   0   0   0
 -1,-1  w ......j.......   0   0   0   0   0   0   0 800   0   0   0   0   0   0
  1,-1  m ......j...b...   0   0   0   0   0   0   0  40   0   0   0   0   0   0
  3,-1  j .......j......   0   0   0   0   0   0   0   0 104   0   0   0   0   0
  5,-1  b ..............   0   0   0   0   0   0   0   0   0   0   0   0   0   0
  0,0   c ..y...........   0   0   0 200   0   0   0   0   0   0   0   0   0   0
  2,0   + ..g.......j...   0   0   0 200   0   0   0   0   0   0   0  80   0   0
 -5,1   l ..........j...   0   0   0   0   0   0   0   0   0   0   0   0   0   0
    12 sectors
.ps +1
.FI
.s1
.SA "deliver, route, commodity, census, Distribution"
