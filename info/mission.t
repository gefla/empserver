.TH Command MISSION
.NA mission "Assign a mission to a ship/plane/unit"
.LV Basic
.SY "mission <TYPE> <UNITS> <s|o|d|i|e|r|a|c|q> <op sector|.> [<radius>]"
.s1
The mission command is used to assign ships, planes, or land units to
missions. The available missions are:
.NF
   Mission        Ships  Land-Units  Planes
i  interdiction     yes         yes  only "tactical"
s       support      no          no  only "tactical"
o   off support      no          no  only "tactical"
d   def support      no          no  only "tactical"
r       reserve      no         yes  no
e        escort      no          no  only "intercept" or "escort"
a   air defense      no          no  only "intercept"
.FI
You may also use 'q' (query) to check on
the mission of something, or 'c' (clear) to wipe the mission of a land
unit/plane/ship.
.s1
GENERAL CONCEPT
.s1
Missions are things that your planes/land units/ships can do automatically, such
as bombing/shelling. You assign a unit to a mission, and whenever the right
conditions occur, the unit attempts to perform its mission. Note that missions
don't allow a unit to do anything it normally could not do, they simply allow
them to do it in an automatic fashion. In some cases, units performing missions
might not be as 'smart' as they would be if you were operating them personally.
That's the price you pay for having automatic defenses.
.s1
.L "IMPORTANT NOTES"
.s1
Artillery units less than 40% efficient will not fire.
.s1
Planes flying missions other than air defense will be intercepted
normally. (i.e. if you fly over enemy territory, you'll get
intercepted.
.s1
Planes assigned to missions other than air defense are \*QNOT\*U
eligible to intercept normally. For example, if a fighter is assigned
to an escort
or interdiction or support mission, it will \*QNOT\*U rise to intercept
an intruding enemy plane. (planes on air-defense missions will intercept
planes flying in their op-area, in accordance with the air-defense mission,
but will not intercept one outside their op-area, even if it is within
their normal range. Be sure you understand this before using plane missions.)
.s1
Planes armed with nukes cannot be assigned missions.
.s1
"marine" missiles will keep launching to interdict ships until all of
the valuable ships in the fleet are sunk.  A "valuable" ship is one
which cost at least $1000 to build or can carry land units or planes.
For each new missile launched, the most valuable ship according to
(cost to build) * efficiency is targeted.
.s1
Non-marine missiles will keep firing
on a sector until 100 damage is done.
.s1
OPERATIONS SECTORS
.s1
Missions require the designation of an 'op sector', short for operations
sector. This is the center of an area that the mission is focused on.
The op sector may be any sector that is within the unit's range.
(Firing range for ships & land units on interdiction, reaction range
for land units on reserve, flying
range for planes, changeable with range command).
The unit will exert influence in a radius around the op sector.
By default, the radius will be as large as possible, i.e. the biggest range
so that the unit/ship/plane could legally act there.)
For example, a ship with a range of 4 could designate any sector up
to 4 away from it as it's op sector, and would affect anything within
4 of it's op-sector, provided it was also within 4 of the ship.
.s1
If a smaller radius is desired, it may be specified on
the command line. If it is not specified, it will not be prompted for.
.s1
The area affected by a unit is known as that unit's op-area.
.s1
If a '.' is given as the op-sector, the op sector will be the location
of the unit.
.s1
Additionally, if the unit's op-sector is the sector the unit is in, and
the unit moves without losing its mission
status, the operation sector will move with it. (Normally, of course, the
only way for a unit on a mission to move is to be carried on a ship, so
this mostly applies to planes on carriers) Thus, a tactical bomber on a ship
could have an op-area centered on the carrier that moved with the carrier.
.s1
Otherwise, if the unit moves without losing its mission status, the
op sector will stay where it was designated.
.s1
For example, suppose that land unit 4 is an artillery unit, range 5,
located at 0,0.
.s1
.EX "mission land 4 int ."
.s1
This would make the unit interdict anything passing within 5 sectors of
its location.
.s1
.EX "mission land 4 int 4,0"
.s1
This would make the unit interdict anything passing within 5 sectors of
4,0, provided it was also within 5 sectors (the unit's range) of 0,0.
.s1
.EX "mission land 4 int 4,0 2"
.s1
This would make the unit interdict anything passing within 2 sectors of
4,0, provided it was also within 5 sectors (the unit's range) of 0,0.
.s1
The op-sector/op-range concept allows you to restrict a unit's area of
interest to a small, important area, or just let it shoot at anything within
range.
.s1
MOBILITY USE
.s1
When a unit is given a mission, it costs no mobility.  When a unit performs
its mission, it uses the normal mobility costs that it would use to
perform that mission if done by the player.
.s1
A unit stays on its mission until you cancel it, OR the unit acts. Any type
of action not a part of a mission
is sufficient, so marching, navigating, transporting, flying, reconing,
bombing, mining, paratrooping, being attacked, being
paratrooped on, retreating, etc, will all cause a unit to lose its
mission status, but the unit could bomb/shell/etc AS PART OF A MISSION, and
still retain its mission status.
Also, land units with a reserve
mission can react to a threatened sector and fight, and (assuming they win)
return to their original sector without losing their reserve status.
.s1
Note that fortifying a unit does not affect it's mission status.
.s1
DEFENDERS
.s1
The defender's planes will intercept planes flying missions.
His ships/forts/units, however, will \*QNOT\*U fire at ships/forts/units
firing on a mission.
.s1
INTERDICTION MISSIONS (ships, planes, land units)
.s1
When giving a unit a interdiction mission, you designate an op sector. From
then on, whenever an enemy (defined as something belonging to a nation which
you are at war with) moves in a sector in that unit's op-area,
the unit will try to shell or bomb it (as appropriate). Almost any kind of
movement, except for distribution movement during an update, and the tiny
amounts moved by the supply routines is enough.
.s1
For example, an enemy land unit marching up to the front would be
shelled/bombed.
An enemy moving commodities around would have the commodities automatically
shelled/bombed. A ship navigating into a sea sector would be shelled or bombed,
depending on it's type (obviously, only depth-charge or ASW capable units would
affect a submarine, and a submarine would only affect things it could torpedo)
In general, units can only affect things they could normally affect. For
example, if the enemy was transporting a plane along a highway, and you had an
artillery unit and a light bomber interdicting the highway sector, the land
unit would fire and the bomber would fly there and bomb, and the total damage
would be applied to the plane being transported. If it was still alive, it
might be moved to another sector, there to be bombed/shelled again (assuming the
new sector was also in both units' op-areas) If a different unit later moved
in, it would be shelled/bombed. This happens as long as the interdicting units
have mobility left.
.s1
Damage done is divided amongst all units/ships/commodities moving. For
example, if 6 ships were moving, and the interdiction mission did 18 points of
damage, each ship would take 3.
.s1
Note that only ships with sonar and ASW planes can interdict subs. Also, the
units may or may not detect the subs, so it's kind of random. The sub also
has a chance of escaping the damage, depending on the visibility of the sub.
.s1
Only planes with the 'tactical' ability can fly interdiction missions.
Only planes with 'ASW' ability will fly vs. subs. Planes with 'ASW'
ability will not fly interdiction vs. other types of
ships/planes/units/commods.  Subs have a chance of sneaking through
ASW air cover unnoticed, and ASW planes will respond to any
non-Friendly sub movement.  Depending on the visibility of the sub,
the ASW plane may or may not identify the type and/or owner of the
sub.

Interdiction has no effect on flying planes. Interdiction does not
affect loading/unloading.  It will also cause some damage to the
sector that the commodities were moving into.
.s1
Note also that forts will fire at enemy ships coming within
range. You don't have to set anything for this, it
happens automatically. (Forts firing at enemy ships naving happens before
ships/planes/units interdicting ships, so if the enemy is using an area a
lot, a fort will hurt him without costing you mob from planes)
.s1
SUPPORT MISSIONS (planes only)
.s1
A support mission allows your planes to automatically bomb the enemy.
Planes with offensive support missions will bomb enemy sectors that you
attack. Planes with defensive support missions will bomb enemy troops
attacking your sector, adding to your defensive strength. Planes with
support orders will do both.
.s1
When giving a plane a support mission, you designate an op sector. From then on,
whenever a sector owned by the plane's owner (or by an ally of the plane-owner,
if the ally is also at war with the attacker)
in that plane's op-area is
attacked, the plane will attempt to fly to that sector and give ground support,
if it has support or defensive support orders.
.s1
If the plane's owner (or an ally of the plane's owner, if the ally is
also at war with the country attacked)
attacks a sector in that plane's op-area belonging to a
country that the plane owner is at war with, the plane will
also attempt to fly there and give offensive support if it has
support or offensive support orders.
.s1
(see info attack for more information on how support works in land combat)
.s1
Only planes with the 'tactical' ability can fly support missions.
.s1
ESCORT MISSIONS (planes only)
.s1
When given an escort mission, an escort or intercept capable plane will attempt
to escort any plane flying a support or interdict mission from the
same airport into its op-area.
If even 1 plane flies a support or interdiction mission from an airport, all
planes on escort duty at that airport will attempt to escort. Planes with escort
duty will NOT attempt to escort planes taking off from other airports, so it
pays to put your bombing planes and escorting planes together.
.s1
Only planes with the 'escort' or 'intercept' abilities can fly escort missions.
.s1
AIR DEFENSE MISSIONS (planes only)
.s1
Putting a plane on an air defense mission modifies where the plane
intercepts.  Without a mission, it intercepts over own sectors,
surface ships and land units.  With an air defense mission, it
intercepts over any sector in its op area.
.s1
.L Example
Groonland wants to run a recon flight over Bannannarama. Joeland has a
jet fighter with an air defense mission and an op-area covering part of
Bannannarama. The 'j' sector belongs to Joeland, and is the base for the
fighter. The 'b' sectors belong to Bannannarama, which has 1 fighter
in the center sector. The 'g' sector belongs to Groonland. Assume that
all countries are mutually at war.
.s1
The Joeland fighter's op area is centered on the 'B', radius 2.
.NF
 . . . . . . . . . . . . . . . . . 
  . . . . . j . . . . . b b . . . .
 . . . . . . . . . . . b B b . . . 
  . . . . . . . . . . . b b . . . .
 . . . . . . . . . g . . . . . . . 
.FI
.s1
The Groonland player enters a path of 'uujjjbgbggh'.
The first sectors entered is sea, and is not in the Joeland
aircraft's op-area, so nothing happens.
.s1
Next, the Groonland plane enters the sector adjacent to Bannannarama.
This is within the Joeland aircraft's op-area, so it flies to intercept.
It takes the shortest path there, and fights the Groonland plane.
Let's say that both take 20 points of damage, and neither aborts.
.s1
Next, the Groonland plane overflies a Bannannarama sector.  It gets
intercepted by Joeland's fighter, because the sector is in its
op-area, and by Bannannarama's fighter, because the sector belongs to
Bannannarama.  And so forth.
.s1
A good use of this mission is to provide a wider CAP around a carrier,
so that enemy planes get intercepted several times on the way to the
carrier, rather than just once, in the carrier's sector.
.s1
Another use is to provide air-cover for an ally who has temporarily
lost his planes.
.s1
RESERVE MISSIONS (land units only)
.s1
A land unit on a reserve mission may react to enemy attacks within its
op-area.  See info \*QAttacking\*U for more on land unit reactions.
.s1
Note that units with a reserve mission retain this status even when they
react to a threatened sector and return. This is an exception to the normal
rules on losing your mission status. If, however, the land unit is forced to
retreat from the combat (see land combat for details), it DOES lose its status.
.s1
Note: information gained from planes flying missions is automatically
added to your bmap.
.s1
.SA "Ship-types, Plane-types, Unit-types, attack, bomb, Ships, Planes, LandUnits"
