.TH Command STRENGTH
.NA strength "Report defensive strengths of sectors"
.LV Expert
.SY "strength <SECTS>"
.s1
A strength lists each of your sectors in the specified area headed by:
.NF
DEFENSE STRENGTH               land  sect   sector  reacting    total
  sect       eff  mil  units  mines  mult  defense     units  defense
.FI
These columns represent:
.s1
.in \w'reacting\0\0'u
.L sect
the x and y coordinates of the sector
.L eff
the efficiency of the sector (affects all benefits
of a designated sector except mobility units)
.L mil
the number of military troops
.L units
the total defensive strength of units in the sector
.L land mines
the number of land mines in the sector.  If you do not completely own
the sector, then a '?' will appear here.
.L sect mult
the defensive multiplier of the sector including the land mine bonus
.L sector defense
(mil + units) * mult
.L reacting units
the total strengths of all supplied mobile reacting units in range
.L total defense
(sector defense) + (reacting units)
.in \\n(in
.s1
For example:
.EX str #1
.NF
Mon Oct 23 22:44:16 1995
DEFENSE STRENGTH               land  sect   sector  reacting    total
  sect       eff  mil  units  mines  mult  defense     units  defense
   5,-5   * 100%   95                1.00       95        48      143
   4,-4   c 100%                     4.00        0       192      192
   6,-4   f   0%   78     48         2.00      252                252
  -1,-3   c 100%                     4.00        0        80       80
   1,-3   c 100%                     4.00        0        80       80
   3,-3   c 100%                     4.00        0        80       80
   7,-3   ! 100%   53            90  2.80      148       134      282
7 sectors
.FI
.SA "attack, assault, Land-units, Combat"
