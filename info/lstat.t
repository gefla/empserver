.TH Command LSTAT
.NA lstat "Report statistics of unit, army or units in a given area"
.LV Basic
.SY "lstat [<UNIT/ARMY> | <SECTS>]"
The lstat command is a report of the varying statistics of your units and
lists the info in a readable format.  Since your units' statistics vary by
tech level, this is a very useful way to see what the attack, defense, etc.
modifiers are on a per-unit basis.
.s1
The <UNIT/ARMY> and <SECTS> arguments are provided in case you only
wish to look at one unit or one army or all units within a given area.
.s1
Lstat expects an argument.  To see every unit you own, give it
the argument '*', i.e. type "lstat *".
.s1
The report format contains the following fields:
.s1
.in \w'unit-type\0\0'u
.L lnd#
the unit number
.L unit-type
the type of unit
.L x,y
the unit's current location (relative to your origin),
.L eff
the unit's efficiency,
.L tech
the tech level of the unit
.L att
the attack multiplier of the unit
.L def
the defense multiplier of the unit
.L vul
the vulnerablilty of the unit (see "info Damage")
.L spd
the speed of the unit (see "info Mobility")
.L vis
the visibility of the unit
.L spy
the distance a unit can see
.L rad
the maximum reaction radius of the unit
.L rng
twice the distance the guns can fire,
The range formula is (rng/2) * ((tech + 50) / (tech + 200))
.L acc
the firing accuracy of the unit
.L fir
the number of guns that fire when the unit fires
.L amm
the amount of ammunition the unit uses per shot fired
.L aaf
the amount of anti-aircraft fire the unit produces
.in
.s1
For example:
.EX lstat *
.NF
                                                      s  v  s  r  r  a  f  a  a
                                                      p  i  p  a  n  c  i  m  a
lnd# unit-type           x,y    eff tech att def vul  d  s  y  d  g  c  r  m  f
   0 cav   cavalry       1,-1   100% 992 3.4 1.6  54 52 18  4  3  0  0  0  0  0
   1 cav   cavalry       0,0    100% 968 3.4 1.6  55 51 18  4  3  0  0  0  0  0
   2 cav   cavalry       0,0    100%  40 1.9 0.9  74 36 18  4  3  0  0  0  0  0
3 units
.FI
.s1
.SA "land, upgrade, build, cargo, army, march, attack, assault, lload, llookout, scrap, lmine, LandUnits"
