.TH Command RETREAT
.NA retreat "Set retreat conditions/paths for ships or fleets"
.LV Expert
.SY "retreat [<SHIP/FLEET> | <SECTS>] [<PATH>] [<CONDITIONS>]"
.SY "lretreat [<UNIT/ARMY> | <SECTS>] [<PATH>] [<CONDITIONS>]"
The retreat command allows you to examine or modify the retreat 
paths and conditions of your ships or units. The following description
is for ships. Land units are exactly the same, except of course that
some of the retreat conditions (being sonared, depth charged, etc) don't apply.
.s1
The <SHIP/FLEET> and <SECTS> arguments are provided to specify which
ships to view or give orders for. If a fleet designation is given when
specifying orders, the orders apply to all ships within that fleet.
.s1
Retreat expects an argument.  To see every ship you own, give it
the argument '*', or type "retreat *".
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
.L path
the ship's current retreat path.
.L as_flt?
whether or not the ship's retreat path is a fleet retreat path
.L flags
what conditions the ship will retreat under
.in
.s1
For example:
.EX retreat *
.NF
shp#     ship type       x,y   fl path       as flt? flags
   0 battleship 4        2,0      jjuuj              I
1 ship
.FI
.s1
.L Overview
.s1
A player may give retreat orders for a ship or a fleet. Retreat 
orders include a set of conditions that determine when the fleet/ship
will retreat, and a path that the fleet/ship will follow. When a condition
is met for a particular ship, the \*Qas flt?\*U field is checked. If it
is Yes, then every ship in that fleet with fleet retreat orders retreats
along the specified path. If it is no, then the retreat orders apply to 
that ship only, and only that ship retreats.
.s1
When a ship is added to a fleet, it is given the retreat orders of that
fleet, if any exist.
.s1
Retreat orders are wiped when a ship navigates.
.s1
Retreat conditions at present include:
.nf

		Condition	Meaning
		I		Retreat when the ship is injured,
				i.e. whenever the ship is damaged
				by fire, bombs, or torping.
		T		Retreat when a sub torpedos or tries
				to torpedo the ship.
		B		Retreat when a plane bombs or tries
				to bomb the ship.
		S		Retreat when the ship detects a sonar
				ping.
		D		Retreat when the ship is depth-charged.
		H		Retreat when helpless. A ship is helpless
				when it is fired upon, and no friendly
				ships/sectors (including the ship itself)
				are able to fire back at the aggressor.
                U               Retreat upon a failed boarding attempt.
		C		Clear the flags
.fi
.s1
For example, if the battleship above was fired at, it would attempt to
retreat 2 sectors, jj, using the normal amount of mobility for moving
2 sectors. It would then look like:
.NF
shp#     ship type       x,y   fl path       as flt? flags
   0 battleship 4        2,0      uuj                I
1 ship
.FI
.s1
.SA "ship, fleetadd, Ships"
