.TH Command RECON
.NA recon "Use planes to take a look around"
.LV Expert
.SY "recon <SPYPLANES> <ESCORTS> <SECT> route|destination"
The \*Qrecon\*U command is used to gather intelligence about sectors
that the spy mission overflies.
The planes take off from their respective airports, assemble, and
then fly the mission and land in the destination sector.
.s1
<SPYPLANES> represents a list of planes which are to be performing the
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
The assembly point must be owned by you, and must not be more than
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
Information given by recon is not totally accurate.
.s1
Recon missions can be intercepted just like any other mission.
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
Note that a plane must be at least 40% efficient before it can leave
the ground.
.s1
.SA "Plane-types, bomb, fly, paradrop, drop, sweep, Mobility, Planes, Detection"
