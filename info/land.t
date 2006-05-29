.TH Command LAND
.NA land "Report status of land units"
.LV Basic
.SY "land <UNITS>"
The land unit report command is a census of your land units.  For
example:
.EX land *
.NF
   # unit type          x,y   a  eff mil frt  mu  fd tch retr rd xl ln carry
   0 cav   cavalry      1,-1    100%  10   0 127  12 992  75%  3  0  0
   1 cav   cavalry      0,0     100%   0   0 127  12 968  75%  3  0  0
   2 cav   cavalry      0,0     100%  10   0 127  12  40  75%  3  0  0
3 units
.FI
.s1
The report format contains the following fields:
.s1
.in \w'unit type\0\0'u
.L #
the unit number
.L "unit type"
the type of land unit; \*Qcavalry\*U, \*Qengineer\*U, etc.,
.L x,y
the land unit's current location,
.L a
the army designation letter (set by \*Qarmy\*U command),
.L eff
the land unit's efficiency,
.L mil
the number of mil currently in the land unit,
.L frt
the land unit's fortification level,
.L mu
the mobility of the land unit,
.L fd
the amount of food on board,
.L fl
the amount of fuel on board (only if the FUEL option is enabled),
.L tch
the tech level of the land unit,
.L retr
the retreat percentage of the land unit (see info \*morale\*U),
.L rd
the land unit's reaction radius (see info \*Qfortify\*U),
.L xl
the number of extra-light planes on board,
.L ln
the number of land units on board,
.L carry
the ship or land unit the carrying the land unit, if any.
.in
.s1
.SA "lstat, upgrade, build, cargo, army, march, attack, assault, lload, llookout, scrap, lmine, LandUnits"
