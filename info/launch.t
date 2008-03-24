.TH Command LAUNCH
.NA launch "Launch missiles or satellites"
.LV Expert
.SY "launch <PLANES> <SECT|SHIP>"
The launch command is used to launch missiles (or satellites)
from sectors or submarines owned by you.
.s1
There are 5 classes of missiles:
.NF
1. "marine" missiles may only be fired at ships.  They may be given
   "interdiction" mission.
2. "tactical" missiles (without "marine" capability) may only be fired
   against sectors.  They may be given missions.  If they fire on a
   mission, then only enough missiles will fire to cause 100 damage.
3. "intercept" missiles can't be launched manually.  They
   automatically intercept planes.
4. "satellite" missiles may only be fired against satellites in orbit.
   They will automatically attempt to intercept AT_WAR satellites
   being launched into orbit.
5. "SDI" (anti-ballistic-missiles) can't be launched manually.  They
   automatically intercept incoming "tactical" (non-"marine")
   missiles.

As well, there are "satellites" which are like missiles in that they
can be launched, but unlike missiles in that they don't get destroyed
as soon as they are launched.
.FI
.s1
In certain cases, a missile will require petrol or shells to be in the
sector before it can be launched.  See 'show plane stats' to find out
how much fuel the missile requires.  The 'load' value in 'show plane
stats' specifies how many shells the missile requires.  Note that if
the missile is armed with a nucelar warhead, then it does not require
any shells.
.s1
Once a satellite has been launched, it can not be moved again.
.s1
Note here that it is much safer to use 100% missiles than those which
are less efficient,
and missiles below 60% cannot be launched.
.s1
The formula for the chance that a missile hits its target is in
\*Qinfo Hitchance\*U.
.s1
In the case of non-marine tactical missiles, if there are any HOSTILE
abm's within range of the target sector, then two of them will fire in
an attempt to intercept the incoming missile.  
.s1
Similarly, two AT_WAR a-sat's will attempt to intercept any satellites
you attempt to launch within their range.  Also, if the sector you are
launching your satellite at is owned by a non-allied country, then
they will get a telegram informing them of the positioning of the
satellite.  See below for details of a satellite's orbit path.
.s1
When launching missiles, there is a 
% chance equal to (5 + 100 - efficiency) * (1 - techfactor) that the
missile will blow up on
the launching pad instead of launching.
.s1
Missiles launched from submarines are anonymous. The victim
is not notified of the identity of the launching country.
.s1
Missiles on interdiction mission have the same chance of hitting and
do the same damage as if they were launched by hand.  Also, any
nuclear armed missiles will detonate if they launch on an interdiction
mission.  You specify whether the nuclear warhead will airburst or
groundburst when you arm the missiles (see info arm).
In order to better destroy missiles on the ground (or in their silos),
the ground-burst technique is used.
Ground-bursts are much less effective against large area targets,
but do massive damage to the sector it impacts,
possibly knocking out hardened missiles
before they can be used against you.
In the parlance of nuclear targeting doctrine,
this is known as \*Qcounter-force\*U targeting.
Air-bursts are used in order to kill large numbers of civilians,
to knock out industry (by flattening the factories),
to take out lightly armored military targets in a large area,
and to generally wreak havoc.
Nuclear strategists call this a \*Qcounter-value\*U technique.
.s1
See Also \*QOn Thermonuclear War\*U by Herman Kahn.
.s1
When you launch a satellite, you will be asked "Geostationary orbit?".
If you answer "y", then your satellite will stay where you launch it.
If, however, you answer "n", then your satellite will move along 5% of
a geodesic path around the world every update.  The path the satellite
follows looks like this:
.NF
+----+
| /  |
|/   |
|   /|
|  / |
| /  |
|/   |
|   /|
|  / |
+----+
.FI

That is, start in the middle, move northeast until you hit the edge,
then continue from the west edge near the top and go northeast until
you go off the top of the map, then continue at the bottom going
northeast until you go off the eastern edge again, and continue from
the western edge near the bottom until you get back to the centre.
.s1
If a foreign satellite moves over your territory durring an update, then you
will get a message like "Fodderland satellite spotted over 12,24" as a
part of your update BULLETIN.
.s1
.SA "build, plane, satellite, Nuke-types, relations, Planes, Nukes"
