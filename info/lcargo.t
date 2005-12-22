.TH Command LCARGO
.NA lcargo "List the commodities on board your units"
.LV Basic
.SY "cargo [<SHIP/FLEET> | <SECTS>]"
.SY "lcargo [<UNIT/ARMY> | <SECTS>]"
The cargo command lists the commodities on board your ships 
in readable format. The lcargo command does the same for land units.
.s1
The <SHIP/FLEET>, <UNIT/ARMY>, and <SECTS> arguments are
provided in case you only wish to look at one ship/unit
or one fleet/army or all ships/units within a given area.
.s1
The report format contains the following fields:
.s1
.in \w'ship type\0\0'u
.L shp#
the ship number, or
.L lnd#
the unit number,
.L "ship type"
the type of ship; \*Qpatrol boat\*U, \*Qsubmarine\*U, etc, or
.L "unit type"
the type of unit; \*Qwar band\*U, \*Qengineer\*U, etc,
.L x,y
the ship/unit's current location (relative to your capital),
.L flt
the fleet/army designation letter (set by \*Qfleetadd/army\*U command),
.L eff
the ship/unit's efficiency,
.L sh
the number of shells on board,
.L gun
the number of guns on board
.L pet
the amount of petroleum on board
.L irn
the amount of iron on board
.L dst
the amount of gold dust on board
.L bar
the number of gold bars on board
.L oil
the amount of oil on board
.L lcm
the amount of light construction materials on board
.L hcm
the amount of heavy construction materials on board
.L rad
the amount of radioactive material on board
.in
.s1
For example:
.EX cargo K
.NF
shp#     ship type      x,y   flt eff   sh gun pet irn dst bar oil lcm hcm rad
  86 aircraft carrier -29,-51  K  100% 298   4   0   0   0   0   0   0   0   0
 110 battleship       -29,-51  K  100% 100   8   0   0   0   0   0   0   0   0
 115 battleship       -29,-51  K  100%  60   8   0   0   0   0   0   0   0   0
 181 destroyer        -27,-35  K  100%  40   4   0   0   0   0   0   0   0   0
 182 destroyer        -27,-35  K  100%  40   4   0   0   0   0   0   0   0   0
 183 destroyer        -27,-35  K  100%  40   4   0   0   0   0   0   0   0   0
6 ships
.FI
.s1
.SA "land, ship, build, march, navigate, load, lload, fleetadd, army, LandUnits"
