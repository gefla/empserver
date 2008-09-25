.TH Command FLY
.NA fly "Fly planes from sector/ship to sector/ship and bomb or snoop"
.LV Basic
.SY "fly <TRANSPORTS> <ESCORTS> <SECT> <PATH|DESTINATION> <COMMODITY>"
The \*Qfly\*U command is used to move planes and commodities from
the starting sector(s), and ending up in the target sector.
.s1
<TRANSPORTS> represents a list of planes which are to travel
to the target sector.
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
<COMMODITY> is the name of a commodity which is to be carried along
on all of the planes which have a transport capacity.
If nothing is specified, then nothing will be carried.
.s1
Route is a normal empire path specification. You may also give a
destination sector on the command line, or at any time while giving
the route, and empire will use the best path from the current sector
to the desired destination sector.
.s1
When getting a path interactively, empire will show you the information
you have (from your bmap) concerning the current area, to help you plot
your course.
.s1
Transport missions can be intercepted just like any other mission.  See
info \*QIntercept\*U.
.s1
If the destination sector is not an airport, then all planes in
the mission must have VTOL capability.
.s1
Note that a plane must be at least 40% efficient before it can leave
the ground.
.s1
.SA "Plane-types, bomb, paradrop, drop, recon, Mobility, Planes, Transportation"
