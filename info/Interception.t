.TH Concept Interception
.NA Interception "How plane interdiction works"
.LV Expert
.s1
Each time an enemy plane (i.e. a plane owned by a country that your are
HOSTILE to or worse) flies into over a sector you own, planes you
have with the intercept ability will intercept them if:

.NF
a) The incoming plane is not stealthy, or is detected.
b) The sector is within 1/2 the interceptor's range (as modified
by rangeedit)
b) They have mobility
c) Their efficiency is 40 or more
d) They are at an appropriate place:
	1) planes without VTOL ability must be at an airport that
		is at least 60% efficient.

	2) planes on ships may only fly if the ship can operate
		that kind of aircraft.

e) There is petrol there equal to 1/2 the plane's fuel rating.
	(rounded down)
.FI
.s1
Before planes scramble to intercept incoming planes, any SAMs in range
will be launched.  One SAM is launched at each incoming plane that
cost at least $1000 to build.  See below for the damage that the SAM
does to the plane it hits.
.s1
Enemy planes will be intercepted by up to 1 more interceptor,
i.e. 1 plane will be intercepted by 2 (if available), 2 by 3,
3 by 4, etc. The highest
numbered planes intercept first. Incoming missiles will only be intercepted
by ABM's
.s1
Once the roster of attackers & intercepters is determined, attackers
and interceptors dogfight. First escorts & interceptors fight,
then planes being escorted & surviving non-aborted interceptors.
.s1
First, an attacker and a defender are chosen, then each plane on each
list fights once.
.s1
.L "Combat values"
.s1
The attacker's combat value is calculated by adding up his planes attack
value, and modifying it for efficiency, bomb-load, and stealth. The attack
value of the plane is multiplied times the plane's efficiency, but cannot
go below 1/2 of the plane-type's base value (as shown by show plane stats).
(If the plane's attack value
is 0, the defensive value is used instead, with a minimum of 1/2 the plane-
type's defensive value)
.s1
Two is subtracted if the attacking plane is carrying bombs.
.s1
If the plane is stealthy, stealth%/25 is added.
.s1
The defender's value is calculated in the same way, except that the
defensive value is always used.
.s1
.L Odds
.s1
Next, an odds ratio is calculated: (att/(def+att))*100.
.s1
.L "Combat Intensity"
.s1
Then, a combat intensity is rolled: (d20+d20+d20+d20)
.s1
.L Combat
For each point of combat intensity (or until both planes are dead),
a d100 is rolled. If the value is less than the odds, the defender
takes a casualty, otherwise the attacker takes a casualty.
.s1
Once all combat rolls have been made, the attacker and defender are
checked for aborting. If a plane is damaged below 80%, then it will
have a chance of aborting.  The chance to abort is (80-plane eff). (Example: a
plane at 70% has a 10% chance to abort.
.s1
Missiles intercepting die after the combat is done.
.L Example
.s1
Suppose that a fighter 2 (30% efficient, tech 154, attack value 7) is
conducting a recon mission. As it enters an enemy controlled sector, a
fighter 1 (100% efficient, tech 131, defense value 4) rises up to fight it.
The attacker's combat value is calculated:
.s1
.NF
1) 7 * (30/100) = 2.1 = 2

2) 2 is less than 1/2 the base attack value of an f2 (6), so the value
is changed to 3

3) The attacking plane is not stealthy. No modifier.

4) The attacking plane is not carrying bombs. No modifier.
.FI
.s1
The defender's defense value, calculated similarly, is 4.
.s1
The odds ratio is (3/(3+4)) = .429 = 43%
.s1
The intensity is randomly determined to be 30. A d100 is rolled 30 times.
Each time that is 43 or less, the defender takes a hit. Each time it is
44 or more, the attacker takes a hit.
.s1
In our example, the attacker takes 17, the defender 13, leaving them
at 13% and 87% efficiency, respectively. The attacker has an 67%
chance to abort, and the defender won't.  If the attacker aborts, the
mission is over.  Else, the attacker flies on to the next sector in
his flight path, perhaps to have combat again.
.s1
.L ABM's
ABM's do not use the normal combat procedure. They simply have a %
chance of destroying an incoming warhead equal to their defensive value.
.s1
If you have land units in the enemy plane's target sector, your plane will
intercept the enemy planes AGAIN. If you have non-sub ships in the enemy plane's
target sector, your planes will intercept them yet again. If you own
the target sector, and have ships & units there, your planes will therefore
intercept them a total of three times in that sector. Note that a 'target
sector' is a sector that is the target of a bombing or para mission. Merely
having ships in a sector will not cause interception of planes flying over.
.s1
.SA "Plane-types, plane, fly, recon, bomb, range, Planes, Missions"
