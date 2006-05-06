.TH Command TRANSPORT
.NA transport "Move planes or nuclear devices around"
.LV Basic
.SY "transport plane <PLANES> <ROUTE|DESTINATION>"
.SY "transport nuke <NUKES> <ROUTE|DESTINATION>"
The transport command is used to move planes and nuclear devices from
one sector to another, along the ground.
.s1
<ROUTE> is a normal empire path specification. You may also give a
<DESTINATION> sector on the command line, or at any time while giving
the route, and empire will use the cheapest path (in terms of mobility)
from the current sector to the destination sector.
.s1
You would move nuke #666 to 8,0 as follows:
.EX transport nuke 666 8,0
.s1
You could move all planes in wing d to -6,0 as follows:
.EX transport plane d -6,0
.s1
Note that while transporting, you may run into any landmines the
enemy has left in your sectors. In that case, the planes being
transported will take damage. See info \*Qlmine\*U for details.
.s1
Also, if your enemy has any units on interdiction mission, then the
planes you're transporting may get hit by them.
.s1
Nuclear warheads are transported in heavily armoured vehicles and are
immune to land mines and interdiction.
.s1
See info \*QMobility\*U for the mobility cost to transport planes and nukes.
.s1
.SA "arm, move, navigate, nuke, Mobility, Planes, Transportation, Moving"
