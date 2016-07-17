.TH Concept Update-sequence
.NA Update-sequence "The order of events at the update"
.LV Expert
.s1
This document gives a rough order of events during the update.
.s1
.nf
1) Prepare
	a) prepare all ships
		a) pay military on board
		b) feed people on board
		c) plague people on board
	b) prepare all planes
		a) pay crew
	c) prepare all land units
		a) pay military on board
		b) feed people on board
		c) plague people on board
	d) prepare all sectors
		a) fallout is checked, if FALLOUT is defined
		b) guerrilla warfare is checked
		c) if there are no civs or mil in the sector, it
			reverts to the deity
		d) taxes are collected from civs & uws; mil are paid.
		e) if the sector is a bank it makes $$ proportional to
			its efficiency
		f) people in non-sanctuary sectors eat
			If not enough is available, the excess people will
			starve off.  No more than 50% of the people
			in a sector can die; the uw's die first, then
			the civs, then the mil.
		g) if there was starvation, the work percentage is set
			to 0, otherwise the work rate rises by
			8+(1-15), max of 100
		h) population grows and is truncated
		i) if the sector doesn't have the plague, see if it
			catches it
			otherwise plague the people
	e) pay for military reserves.

2) Produce
	a) ship maintenance, in order of ship number
	b) plane maintenance, in order of plane number
	c) land unit maintenance, in order of land unit number
	d) sectors, top to bottom, left to right
		a) pay sector maintenance
		b) sectors that are stopped are skipped (see info stop)
		c) first increase eff
		d) then make things
	e) ship building, in order of ship number
	        first increase efficiency, then produce
                stopped ships are started, but not built (see info stop)
	f) plane building, in order of plane number
                stopped planes are started, but not built (see info stop)
	g) land unit building, in order of land unit number
                stopped land units are started, but not built (see info stop)

3) Then, do deliveries for all sectors in the world, row by row, going from
	top to bottom, left to right

4) Then, do exporting via distribution for all sectors in the world, row by
	row, going from top to bottom, left to right

5) Then, do importing via distribution for all sectors in the world, row by
	row, going from top to bottom, left to right
   Also start sectors that are stopped

6) modify tech/res/hap/edu levels & do tech bleed
7) decrease levels due to passing of time
8) update war declarations (for slow war)

9) If MOB_ACCESS is not enabled, mobility is updated as follows:
   a) Add mobility to ships
   b) Add mobility to sectors
   c) Add mobility to planes
   d) Add mobility to land units
.fi
.s1
.SA "Innards, Updates"
