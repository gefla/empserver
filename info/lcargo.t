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
the type of ship; \*Qpt\*U, \*Qfb\*U, etc, or
.L "unit type"
the type of unit; \*Qart\*U, \*Qeng\*U, etc,
.L x,y
the ship/unit's current location,
.L flt
the fleet/army designation letter (set by \*Qfleetadd/army\*U command),
.L eff
the ship/unit's efficiency,
.L civ
the number of civilians on board,
.L mil
the number of military on board,
.L uw
the number of uncompensated workers on board,
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
.EX cargo F
.NF
shp#         x,y   flt eff  civ mil  uw  sh gun pet irn dst bar oil lcm hcm rad
   3 dd      8,-14  F   86%   0   2   0  34   4   0   0   0   0   0   0   0   0
   4 fb     -2,14   F  100%  10  10   0   0   0  10   0   0   0  10   0   0   0
   7 dd      8,-14  F  100%   0  60   0  40   4   0   0   0   0   0   0   0   0
  10 fb      5,-15  F  100%   1   0   0   0   0   0   0   0   0   0   0   0   0
  11 fb      3,-15  F   20%   0   0   0   0   0   0   0   0   0   0   0   0   0
  13 bb      4,-16  F   80%   0  10   0 100   4   0   0   0   0   0   0   0   0
6 ships
.FI
.s1
.SA "land, ship, build, march, navigate, load, lload, fleetadd, army, LandUnits"
