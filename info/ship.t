.TH Command SHIP
.NA ship "Report status of ship, fleet or ships in a given area"
.LV Basic
.SY "ship [<SHIP/FLEET> | <SECTS>]"
The ship report command is a census of your ships and lists all the
info available in readable format.
.s1
The <SHIP/FLEET> and <SECTS> arguments are provided in case you only
wish to look at one ship
or one fleet or all ships within a given area.
.s1
Ship expects an argument.  To see every ship you own, give it
the argument '*', or type "ship *".
.s1
The report format contains the following fields:
.s1
.in \w'shiptype\0\0'u
.L shp#
the ship number
.L shiptype
the type of ship; \*Qpatrol boat\*U, \*Qsubmarine\*U, etc,
.L x,y
the ship's current location (relative to your capital),
.L flt
the fleet designation letter (set by \*Qfleetadd\*U command),
.L eff
the ship's efficiency,
.L civ
the number of civilians on board
.L mil
mil is the number of military on board
.L uw
the number of uncompensated workers on board,
.L fd
the amount of food on board
.L pn
the number of planes on the ship
.L he
the number of helicopters on board (if the SHIPCHOPPERS option is enabled)
.L xl
the number of extra-light planes on board (if the XLIGHT option is enabled)
.L ln
the number of land units on board
.L fuel
the amount of fuel on board (if the FUEL option is enabled)
.L mu
the mobility of the ship
.L tech
the tech level of the ship
.in
.s1
For example:
.EX ship *
.NF
shp#     ship type       x,y   fl  eff civ mil  uw  fd pn he xl ln mob fuel tech
   0 dd  destroyer       1,1      100%   0   0   0   0  0  0  0  0 127   30 140
   1 bb  battleship      1,1      100%   0   0   0   0  0  0  0  0 120   50 200
2 ships
.FI
.s1
.SA "sstat, build, cargo, fleetadd, navigate, load, lookout, scrap, sonar, torpedo, upgrade, land, Ships"
