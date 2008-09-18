.TH Command BOMB
.NA bomb "Dispose of inconvenient enemy assets with air power."
.LV Basic
.SY "bomb <BOMBERS> <ESCORTS> mission-type <SECT> route|destination"
The \*Qbomb\*U command is used to rain destruction upon helpless
enemy ships, planes, land units, and sectors.
It represents a complete mission for one or more bombers and escorts
taking off from one place and returning to base at the end of the mission.
.s1
<BOMBERS> represents a list of planes which can carry bombs.
Only planes which have a bomb bay, belong in a sector which is
stocked with gas and shells, and have sufficient mobility will
be successfully selected for the mission.
.s1
<ESCORTS> represent a list of fighter planes which are capable of escorting
the bombers all the way to the target.
To be selected, escorts must have fuel and mobility.
.s1
Mission-type must be one of "pinpoint" or "strategic".
Each mission has different objectives for the planes performing it.
Pinpoint missions allow bombers to attack ships, sector efficiency,
commodities in sectors, land units, and planes on the ground in the
target sector.
Strategic missions damage all commodities in the entire target sector,
while not damaging any ships resident (similar to the
\*Qfire\*U command).
If a plane is armed with a nuclear warhead, then it will drop the bomb
rather than using conventional weapons.
Nuclear devices damage everything in the sector; planes, ships, and
commodities.
Only missiles in hardened silos can avoid damage from a nuclear mission.
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
When getting a path interactively, empire will show you a small map
(from your bmap) concerning the current area, to help you plot
your course.
.s1
Light bombers (with the tactical capability) are used primarily for pinpoint
bombing.  They perform as well as fighters do for strategic bombing.
Heavy bombers (with the bomber capability), while they can perform pinpoint
bombing, are notoriously inaccurate and in general miss what they are aiming
at.  They are used mainly for strategic bombing.
.s1
During the course of a mission,
your planes may fly over someone else's land.
If you fly over land owned by another country,
and this country is either hostile or in any war state (see info relation)
with you, fighter planes from the enemy country will scramble
and attempt to intercept your force.
If the country is simply neutral,
he will be notified that his radar tracked your planes
as they flew over his country.
If the country is allied, then nothing will occur.
.s1
If you manage to fight though the enemy fighters and arrive
at the target sector, your force must still contend with
any flak guns which your enemy has stationed there.
What's more, any enemy ship in the target sector will also
shoot at each of your bombers before any attacks are performed.
Any enemy land units with the 'flak' ability will also fire at
all your planes at this time.
.s1
A well-equipped fleet of battleships in a heavily-defended harbor
can be very risky to attack, even though hitting ships in harbor is
like shooting fish in a barrel.
.s1
When bombing ships/planes/units, entering a '~' character will cause a
particular plane not to bomb. Other planes on the mission will still
be asked for their targets.
.s1
If your plane has anti-submarine
(ASW) capabilities, you will be told when you arrive in a sector whether
it has submarines in it. If you then choose to pin-bomb ships, each plane
in turn will attempt to find submarines (this ability varies with the plane's
accuracy rating).  Each plane can only bomb the subs it can find. (It
is assumed
that planes on anti-sub missions split up to cover the area)
.s1
Planes have differing air-to-air combat abilities, differing ranges,
and load-carrying capacities.
Some airplanes don't need runways, but can operate out of any
sector which has fuel in it.
.s1
When you pin-bomb a land unit, it will fire flak at the plane bombing it,
unless
it has an AA fire rating of 0.
.s1
In the case of pinpoint bombing, see \*Qinfo Hitchance\*U for the formula
determining whether a plane hits its target.
.s1
Note that a plane must be at least 40% efficient before it can leave
the ground.
.s1
.SA "Plane-types, build, fly, paradrop, drop, recon, relations, Combat, Mobility, Damage, Planes"
