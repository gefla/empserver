.TH Command SWEEP
.NA sweep "Use planes to remove mines from sea sectors"
.LV Expert
.SY "sweep <PLANES> <ESCORTS> <SECT> route|destination"
The \*Qsweep\*U command is identical to the recon command, except
that if you use planes capable of minesweeping, they will sweep
mines from sectors along their route.
.s1
<PLANES> represents a list of planes which are to be performing the
intelligence gathering operation.
Only planes which are provided with gas, and have sufficient mobility will
be successfully selected for the mission.
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
Sweep/recon missions can be intercepted just like any other mission.
If the destination sector is not an airport, then all planes in
the mission must have V/STOL capability.
.s1
Planes with the ASW ability will
perform a sonar search on each sector as they fly through it.
Each sub contact will be reported only once, giving the sector and number
of the sub, from the sector in which it
is first detected (which may not be the sector the sub is in, as plane
sonar has a range dependent on acc, and a chance of finding a sub of
((100-acc)-(4-sub visib)*10))+((100-effic)/5) percent.
The sub being sonared may detect the sonar ping. 
.s1
Planes with the sweep ability will
automatically attempt to sweep mines in sea or harbor sectors they overfly.
Their chance of sweeping is (100-acc), and they can only sweep 1 mine per
overflight of the sector.
.s1
Note that a plane must be at least 40% efficient before it can leave
the ground.
.s1
.SA "Plane-types, bomb, fly, paradrop, drop, recon, Planes"
