.TH Command LRETREAT
.NA lretreat "Set retreat conditions/paths for units or armies"
.LV Expert
.SY "lretreat <UNIT/ARMY> <PATH> <CONDITIONS>"
.SY "lretreat <UNIT/ARMY> q"
The lretreat command allows you to examine or modify the retreat
paths and conditions of your land units.
.s1
The first argument selects land units to view or give orders for.
If an army designation is given when specifying orders, the orders
apply to all members of that army.
.s1
With 'q' instead of a retreat path, the current retreat paths and
conditions will be listed.
The report format contains the following fields:
.s1
.in \w'unit type\0\0'u
.L lnd#
the unit number
.L "unit type"
the type of land unit; \*Qcavalry\*U, \*Qengineer\*U, etc.,
.L x,y
the land unit's current location,
.L a
the army designation letter (set by \*Qarmy\*U command),
.L path
the current retreat path,
.L as army?
whether these are army retreat orders,
.L flags
what conditions will trigger retreat.
.in
.s1
For example:
.EX lretreat * q
.NF
lnd#     unit type       x,y   ar path       as army? flags
   0 cav  cavalry        2,0      jjuuj               I
1 unit
.FI
.s1
A player may give retreat orders for a land unit or an army.  Retreat
orders include a set of conditions that determine when the army/unit
will retreat, and a path that the army/unit will follow.  When a condition
is met for a particular land unit, the \*Qas army?\*U field is checked. If it
is Yes, then every unit in that army with army retreat orders retreats
along the specified path. If it is no, then the retreat orders apply to
that unit only, and only that unit retreats.
.s1
When a land unit is added to an army, it is given the retreat orders
of the first unit in that army that has army retreat orders and is in
the same sector, if any exist.
.s1
Retreat orders are wiped when a land unit marches.
.s1
Retreat conditions are:
.nf

	Condition	Meaning
	i		Retreat when the unit is injured,
			i.e. whenever the unit is damaged by gun fire
			or bombs.
	b		Retreat when a plane bombs or tries to bomb
			the unit.
	h		Retreat when helpless --- a unit is helpless
			when it is fired upon, and no friendly
			units/sectors (including the unit itself)
			are able to fire back at the aggressor.
	c		Cancel retreat order
.fi
.s1
For example, if the cavalry above was fired at, it would attempt to
retreat 2 sectors, jj, using the normal amount of mobility for moving
2 sectors. It would then look like:
.NF
lnd#     unit type       x,y   ar path       as army? flags
   0 cav  cavalry        2,0      uuj                 I
1 unit
.FI
.s1
.SA "land, army, LandUnits"
