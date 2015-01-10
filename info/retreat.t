.TH Command RETREAT
.NA retreat "Set retreat conditions/paths for ships or fleets"
.LV Expert
.SY "retreat <SHIP/FLEET> <PATH> <CONDITIONS>"
.SY "retreat <SHIP/FLEET> q"
The retreat command allows you to examine or modify the retreat
paths and conditions of your ships.
.s1
The first argument selects ships to view or give orders for.
If a fleet designation is given when specifying orders, the orders
apply to all members of that fleet.
.s1
With 'q' instead of a retreat path, the current retreat paths and
conditions will be listed.
The report format contains the following fields:
.s1
.in \w'ship type\0\0'u
.L shp#
the ship number
.L ship type
the type of ship; \*Qpatrol boat\*U, \*Qsubmarine\*U, etc,
.L x,y
the ship's current location,
.L flt
the fleet designation letter (set by \*Qfleetadd\*U command),
.L path
the current retreat path,
.L as flt?
whether these are fleet retreat orders,
.L flags
what conditions will trigger retreat.
.in
.s1
For example:
.EX retreat * q
.NF
shp#     ship type       x,y   fl path       as flt?  flags
   0 bb   battleship     2,0      jjuuj               i
1 ship
.FI
.s1
A player may give retreat orders for a ship or a fleet. Retreat
orders include a set of conditions that determine when the fleet/ship
will retreat, and a path that the fleet/ship will follow. When a condition
is met for a particular ship, the \*Qas flt?\*U field is checked. If it
is Yes, then every ship in that fleet with fleet retreat orders retreats
along the specified path. If it is no, then the retreat orders apply to
that ship only, and only that ship retreats.
.s1
When a ship is added to a fleet, it is given the retreat orders of the
first ship in that fleet that has fleet retreat orders and is in the
same sector, if any exist.
.s1
Retreat orders are wiped when a ship navigates.
.s1
Retreat conditions are:
.nf

	Condition	Meaning
	i		Retreat when the ship is injured,
			i.e. whenever the ship is damaged by gun fire,
			bombs or torpedoes
	t		Retreat when a sub torpedoes or tries to
			torpedo the ship
	b		Retreat when a plane bombs or tries to bomb
			the ship
	s		Retreat when the ship detects a sonar ping
	d		Retreat when the ship is depth-charged
	h		Retreat when helpless --- a ship is helpless
			when it is fired upon, and no friendly
			ships/sectors (including the ship itself) are
			able to fire back at the aggressor
	u		Retreat upon a failed boarding attempt
	c		Cancel retreat order
.fi
.s1
For example, if the battleship above was fired at, it would attempt to
retreat 2 sectors, jj, using the normal amount of mobility for moving
2 sectors. It would then look like:
.NF
shp#     ship type       x,y   fl path       as flt?  flags
   0 bb   battleship     2,0      uuj                 i
1 ship
.FI
.s1
.SA "ship, fleetadd, Ships"
