.TH Command LEVEL
.NA level "List distribution thresholds"
.LV Basic
.SY "level <SECTS>"
The level report provides information about the various 
distribution thresholds in some or all of the sectors that
you occupy.
.s1
In the syntax of the level command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
A typical usage might be:
.EX level -6:6,-3:3
which would list data for the area extending three sectors
out from the capital in each direction.
.s1
The level report lists each of your sectors with the following heading:
.NF
DISTRIBUTION LEVELS
 sect    dst   civ mil uw food  sh gun pet iron dust bar oil lcm hcm rad
.FI
These columns represent:
.s1
.in \w'path------\0\0'u
.L sect
the x and y coordinates of the sector
.L path------
the route that goods travel to and from the distribution sector
.L dst
the distribution destination for the sector
.L civ
the distribution threshold for civilians in that sector
.L mil
the distribution threshold for military in that sector
.L uw
the distribution threshold for uncompensated workers in that sector
.L food
the distribution threshold for food in that sector
.L gun
the distribution threshold for guns in that sector
.L sh
the distribution threshold for shells in that sector
.L pet
the distribution threshold for petroleum in that sector
.L iron
the distribution threshold for iron ore in that sector
.L dust
the distribution threshold for gold dust in that sector
.L bar
the distribution threshold for gold bars in that sector
.L oil
the distribution threshold for oil in that sector
.L lcm
the distribution threshold for light construction materials
in that sector
.L hcm
the distribution threshold for heavy construction materials
in that sector
.L rad
the distribution threshold for radioactive materials in that sector
.in
.s1
For example:
.EX level -2,2
.NF
.ps -1
Fri Oct 31 16:58:34 1986
DISTRIBUTION LEVELS
 sect    dst   civ mil  uw food  sh gun pet iron dust bar oil lcm hcm rad
-2,-2 + -2,-2    0   0 201    0   0   0   0    0    0   0   0   0   0   0
   1 sector
.ps +1
.FI
.s1
.SA "distribute, threshold, commodity, census, Distribution"
