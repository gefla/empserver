.TH Command SWEEP
.NA sweep "Use planes to remove mines from sea sectors"
.LV Expert
.SY "sweep <PLANES> <ESCORTS> <SECT> route|destination"
The \*Qsweep\*U command is used to sweep sea mines from the sectors
along the route.
.s1
<PLANES> represents a list of planes which are to perform the
mine-sweeping mission.
Only planes with capability sweep and sufficient mobility, based in a
sector stocked with petrol, will be successfully selected for the
mission.
.s1
<ESCORTS> represent a list of fighter planes which are capable of escorting
the transports all the way to the target.
To be selected, escorts must have fuel and mobility.
.s1
<SECT> represents an assembly point, where all of the planes in the
mission meet before proceeding on to the target sector.
The assembly point must be owned by you or an ally, or you or an ally
must have a ship there.  It must not be more than
four sectors away from any of the planes selected for the mission.
.s1
Route is a normal empire path specification. You may also give a
destination sector on the command line, or at any time while giving
the route, and empire will use the best path from the current sector
to the desired destination sector.
.s1
When getting a path interactively, empire will show you the information
you have (from your bmap) concerning the current area, to help plot
your course.
.s1
Information given by sweep/recon is not totally accurate.
.s1
Sweep missions can be intercepted just like any other mission.  See
info \*QIntercept\*U.
.s1
If the destination sector is not an airport, then all planes in
the mission must have V/STOL capability.
.s1
Planes will
automatically attempt to sweep sea mines in sea sectors they overfly.
Their chance of sweeping is (100-acc), and they can only sweep 1 mine per
overflight of the sector.
.s1
The planes will additionally gather intelligence, just like they do on
a recon mission.  See info \*Qrecon\*U.
.s1
Note that a plane must be at least 40% efficient before it can leave
the ground.
.s1
.SA "Plane-types, bomb, fly, paradrop, drop, recon, Planes"
