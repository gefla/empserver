.TH Command LAND
.NA land "Report status of unit, army or units in a given area"
.LV Basic
.SY "land [<UNIT/ARMY> | <SECTS>]"
The unit report command is a census of your units and lists all the
info available in readable format.
.s1
The <UNIT/ARMY> and <SECTS> arguments are provided in case you only
wish to look at one unit or one army or all units within a given area.
.s1
Land expects an argument.  To see every unit you own, give it
the argument '*', i.e. type "land *".
.s1
The report format contains the following fields:
.s1
.in \w'unit-type\0\0'u
.L #
the unit number
.L unit type
the type of unit; \*Qwar band\*U, \*Qengineer\*U, etc,
.L x,y
the unit's current location (relative to your capital),
.L a
the army designation letter (set by \*Qarmy\*U command),
.L eff
the unit's efficiency,
.L mil
the number of mil currently in the land unit
.L frt
the unit's fortification level (0-127)
.L mu
the mobility of the unit
.L fd
the amount of food on board
.L fuel
the amount of fuel on board (if the FUEL option is enabled)
.L tch
the tech level of the unit
.L retr
the retreat percentage of the unit (see info attack)
.L rd
the unit's reaction radius (see info attack)
.L xl
the number of extra-light planes on board (if the XLIGHT option is enabled)
.L ln
the number of land units on board
.L l/s
the ship or land unit the unit is on, if any
.in
.s1
For example:
.EX land *
.NF
   # unit type            x,y   a  eff mil frt  mu  fd fl tch retr rd xl ln l/s
   0 cav   cavalry        1,-1    100%  10   0 127  12  0 992  75%  3  0  0
   1 cav   cavalry        0,0     100%   0   0 127  12  0 968  75%  3  0  0
   2 cav   cavalry        0,0     100%  10   0 127  12  0  40  75%  3  0  0
3 units
.FI
.s1
.SA "lstat, upgrade, build, cargo, army, march, attack, assault, lload, llookout, scrap, lmine, LandUnits"
