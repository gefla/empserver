.TH Server Bugs
.NA Bugs "Known bugs in the Empire Server"
.LV Expert
.s1
NOTE: This file is dramatically out of date.
.S1
The classification scheme used by report is dumb.
.s1
You can make a sector temporarily useless by filling up all its
fields with delivery and distribution information.
This is useful when an enemy is trying to capture the sector
(his mil don't have room to move in :-)
You have to halt some of the deliveries or distributions
to make room for the military to move in.
(Mostly fixed by changing the number of available fields)
.s1
Warehouses can't distribute all commodities simultaneously,
due to limited fields for this information.
This becomes a problem if you have a countrywide network
of warehouses distributing to each other.
(Mostly fixed by changing the number of available fields)
.s1
You can sometimes build planes in non-airport sectors,
deliberately or accidentally. FIXED
.s1
You can sometimes move small quantities of certain items
from warehouses at no mobility cost, even into mountains
(this is my favorite bug, I'd hate to see it fixed :-)
.s1
Guerrillas don't seem to carry the plague.
.s1
You can get information about enemy sectors
by attacking with 0 military.
This costs 0 BTU's currently
(this is another of my favorite bugs :-)
This is a good way to flood the enemy mailbox with junk mail
at no cost in BTU's. FIXED
.s1
You can sometimes trick someone into paying a huge price for
commodities by changing the price suddenly.
Therefore one should always check prices when buying commodities.
.s1
When two countries are attacking each other simultaneously,
you can sometimes move into a sector he is in the process of attacking.
If you get the timing right,
he will take the sector but you will get it back,
along with all his military.
.s1
If a plane is out to trade, and gets shot down,
it can still be bought until the next update.
If another country builds a new plane
that gets the number of the plane that was shot down,
the new plane will go on the trading market automatically.
Then if that plane is bought,
the money goes to the country whose plane was shot down,
not the country that built the plane.
I stole numerous planes
(including nuclear missiles :-)
this way
(by deliberately putting low numbered planes up for trade,
then having them shot down).
.s1
If a plane has negative mobility, then gets traded, mobility
goes to 0.
.s1
Firing on sectors with land-locked sunken ships does strange things.
.s1
If two countries are cooperating,
its possible to raid an enemy airport and steal the planes
by putting them out to trade.
.s1
You can also strip enemy sectors of commodities using \*Qsell\*U,
if you have military control temporarily.
.s1
One can raid an enemy harbor and use up all the raw materials
by building useless ships. FIXED
.s1
One can make work go back to 100 everywhere in a country by
moving all civilians in low-work sectors onto a bridge,
then collapsing the bridge.
Work then goes to 100 at the next update,
if you leave some mil in the vacated sectors.
Or you can move mil out too,
letting the sector ownership change to the Deity,
then move back in from a 100% working sector,
and work goes immediately to 100.
.s1
Two cooperative countries can move commodities around at no
mobility cost using the market.
.s1
You can depopulate an enemy sector in a raid by converting
all civs to uws and selling them.
This can make it hard for the victim to get back in the game
even with a rapid & successful counterattack. FIXED, convert takes mob now.
.s1
Sometimes sector ownership doesn't change when the last civ starves,
so you may get census information about sectors with 0 civs.
Later this same sector may get its ownership
switched to the Deity without your doing anything. FIXED
.s1
You can only have so much of a commodity in a sector.
If you went over 32,000 and something, units of food, for example,
bizarre things might happen.
I think that has been fixed. FIXED
.s1
Treaty had bugs
(for example, firing on any ships was considered a treaty violation,
not just the ships of the country you had a treaty with).
I think Jeff might have fixed this. FIXED
.s1
Also air attacks were not considered treaty violations.
This may have been fixed. FIXED
.s1
You can collapse enemy bridges by making a lightning raid on
his bridgeheads and redesignating them,
even if you only hold the bridgehead for a short time.
(In this games, bridges work differently, see info build, info bridges")
.s1
You can map out enemy territory by raiding his radar stations.
.s1
Condition checking is very treacherous.
Global commands with conditions are unreliable.
I never figured out exactly what was wrong,
although I think your method of putting conditions
towards the front of the line helped sometimes.
.s1
You can have more than 26 ships in a fleet,
but only the first 26 will move when you navigate the fleet
(I think 26 is the right number, but I'm not certain.
It might be 32).
.s1
\*QLook\*U only spots subs (from destroyers) at a certain distance.
If you are too close you won't see them
(unless you are in the same sector).
.s1
You can only fly as many planes on a mission as
you can fit on the command line
(so low numbered planes have an advantage this way).
USE WINGS
.s1
I never managed to load more than 1 warhead on a missile,
but I didn't have time to investigate this thoroughly. FIXED
.s1
Wilderness sectors that you own don't appear as \*Q?\*U on enemy maps,
so you can sometimes hide information by leaving sectors
as wilderness. FIXED
.s1
When a sector has a visible ship,
radar doesn't show whether the sector is land or sea, just the ship.
This has interesting possibilities for exploitation
(like land-locking a battleship
in your capital in order to deceive the enemy :-)
.s1
I don't think you can land planes on a land-locked aircraft carrier
anymore.
.s1
Its common to mistakenly set the price of a plane or ship incorrectly
so one should check trade after using set.
.s1
Torpedo isn't random, but always works within range
(another of my favorite bugs :-) FIXED
.s1
Transporting nukes costs 0 mobility. FIXED
.s1
The \*Qmust be accepted by\*U date on offered loans is bogus.
.s1
\*QTurn off\*U doesn't stop updates.
.s1
.SA "list, Innards, Server"
