.TH Command SSTAT
.NA sstat "Report statistics of ship, fleet or ships in a given area"
.LV Basic
.SY "sstat [<SHIP/FLEET> | <SECTS>]"
The sstat command is a report of the varying statistics of your ships and
lists the info in a readable format.  Since your ships' statistics vary by
tech level, this is a very useful way to see what the attack, defense, etc.
modifiers are on a per-ship basis.
.s1
The <SHIP/FLEET> and <SECTS> arguments are provided in case you only
wish to look at one ship
or one fleet or all ships within a given area.
.s1
Sstat expects an argument.  To see every ship you own, give it
the argument '*', or type "sstat *".
.s1
The report format contains the following fields:
.s1
.in \w'shiptype\0\0'u
.L shp#
the ship number
.L ship-type
the type of ship; \*Qpatrol boat\*U, \*Qsubmarine\*U, etc,
.L x,y
the ship's current location (relative to your capital),
.L eff
the ship's efficiency,
.L tech
the tech level of the ship
.L def
the armor value of the ship (i.e. how strong is the hull)
.L spd
the speed of the ship
.L vis
the visibility of the ship (i.e. how easy is it to see)
.L rng
twice the distance the guns can fire,
The range formula is (rng/2) * ((tech + 50) / (tech + 200))
.L fir
the number of guns the ship can fire at once
.in
.s1
For example:
.EX ship *
.NF
shp# ship-type                 x,y    eff  tech def spd vis rng fir
   0 dd  destroyer             1,1    100%  140  51  41  19   9   2
   1 bb  battleship            1,1    100%  200 129  35  34  17   7
2 ships
.FI
.s1
.SA "ship, build, cargo, fleetadd, navigate, load, lookout, scrap, sonar, torpedo, upgrade, land, Ships"
