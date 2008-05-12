.TH Command SHIP
.NA ship "Report status of ships"
.LV Basic
.SY "ship <SHIPS>"
The ship report command is a census of your ships.  For example:
.EX ship *
.NF
shp#     ship type       x,y   fl   eff civ mil  uw  fd pn he xl ln mob tech
   0 dd  destroyer       1,1       100%   0   0   0   0  0  0  0  0 127  140
   1 bb  battleship      1,1       100%   0   0   0   0  0  0  0  0 120  200
2 ships
.FI
.s1
The report format contains the following fields:
.s1
.in \w'ship type\0\0'u
.L shp#
the ship number
.L "ship type"
the type of ship; \*Qpatrol boat\*U, \*Qsubmarine\*U, etc.,
.L x,y
the ship's current location,
.L flt
the fleet designation letter (set by \*Qfleetadd\*U command),
.L eff
the ship's efficiency, prefixed by \*Q=\*U if stopped,
.L civ
the number of civilians on board,
.L mil
mil is the number of military on board,
.L uw
the number of uncompensated workers on board,
.L fd
the amount of food on board,
.L pn
the number of planes on the ship,
.L he
the number of helicopters on board,
.L xl
the number of extra-light planes on board,
.L ln
the number of land units on board,
.L mob
the mobility of the ship,
.L tech
the tech level of the ship.
.in
.s1
.SA "sstat, build, cargo, fleetadd, navigate, load, lookout, scrap, sonar, torpedo, upgrade, land, Ships"
