.TH Command LLOOKOUT
.NA llookout "Check from units for other units, planes and/or sector types"
.LV Basic
.SY "lookout <UNIT/ARMY>"
The lookout command allows units to report sightings
of other units, planes, and land sectors.
.s1
Note that non-spy units require military on-board in order to be able
to see anything.
.s1
The various ranges over which lookout is effective
break down into two groups:
.s1
UNIT to UNIT
.s1
.in +0.2i
This range is dependent on both the visibility of the unit
being observed and the visual range of the observing unit.
(see \*Qinfo Unit-types\*U)
.in \\n(in
.s1
UNIT to PLANE
.s1
Planes are treated as if they had a visibility of 10.
.in +0.2i
For example:
.EX lookout 24
.NF
Groonland (#2) jf4 jet fighter 4 #3 @2,-2
Groonland (#2) airfield 100% efficient with approx 50 civ @ 2,-2
Groonland (#2) highway 20% efficient with approx 20 civ @ 3,-1
Groonland (#2) highway 0% efficient with approx 0 civ @ 0,-2
.FI
.s1
.SA "lookout, coastwatch, Unit-types, Sector-types, radar, sonar, LandUnits, Detection"
