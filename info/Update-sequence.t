.TH Concept Update-sequence
.NA Update-sequence "The order of events at the update"
.LV Expert
.s1
This document gives a rough order of events during the update.
.s1
.nf
1) First, prepare all sectors in the world, row by row, going from top
	to bottom, left to right

	a) fallout is checked, if FALLOUT is defined
	b) guerrilla warfare is checked
	c) if the sector doesn't have the plague, see if it catches it
		otherwise plague the people
	d) if there are no civs or mil in the sector, it reverts to the deity
	e) taxes are collected from civs & uws; mil are paid.
	f) if the sector is a bank it makes $$ proportional to its efficiency

2) Then, in order of country #, deal with each country:
	a) pay for military reserves.
	c) ship maintenance
		pay maintenance, produce, then feed and plague people on board
	d) ship building
		ships that are stopped are skipped (see info stop)
		ships execute their orders right after building
		finally, if option SAIL is enabled, follow sail paths
	e) plane maintenance
	f) plane building
		planes that are stopped are skipped (see info stop)
	g) land unit maintenance
		pay maintenance, then feed and plague people on board
	h) land unit building
		land units that are stopped are skipped (see info stop)
	i) produce for all sectors
                a) if the sector is a cap, it costs $1 per etu since
			the last update
                b) people in non-sanctuary sectors eat
                        1) If not enough is available, the sector
                                uses the supply routines to draw more.
                        2) If still not enough is available,
                                the excess people will starve off.
                                a) No more than 50% of the people
                                        in a sect can die;
                                        the uw's die first, then
                                        the civs, then the mil.
                c) if there was starvation, the work percentage is set
                        to 0, otherwise the work rate rises by
                        8+(1-15), max of 100
                d) population grows and is truncated
                e) sectors that are stopped are skipped (see info stop)
                f) first increase eff
                g) then make things

3) Then, do deliveries for all sectors in the world, row by row, going from
	top to bottom, left to right

4) Then, do exporting via distribution for all sectors in the world, row by
	row, going from top to bottom, left to right

5) Then, do importing via distribution for all sectors in the world, row by
	row, going from top to bottom, left to right

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
