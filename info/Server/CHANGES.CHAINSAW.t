.TH Server "Chainsaw Changes"
.NA CHANGES.CHAINSAW "Changes from KSU Empire to Chainsaw code"
.LV Expert
.s1
CHANGES.CHAINSAW.1.0
.s1
Changes to the Chainsaw code in version 1.0
.s1
This file enumerates some of the more important changes between
BBN Empire 1.0 & BSD Empire 1.1.5, KSU, & MERC empire. BBN Empire 1.0
is a superset of the other versions, including all MERC code, which includes
all KSU code, which include the BSD 1.1.5 code.
.s1
.L Bug-fixes & Changes
.s1
Scrap and scuttle will now ask for confirmation before scrapping or scuttling
wing or fleets. The will also ask before scrapping or scuttling ALL planes or
ships (when a '*' is given as an argument)
.s1
Bomb has been changed to allow planes with the P_T (tactical bomber) flag to
bomb, even if they are also cargo planes. This fixes the problem with Zeppelins
not being allowed to bomb.
.s1
Power has been modified to not report on all deity countries (as opposed to just
ignoring country 0)
.s1
The plane functions have been extensively modified to calculate and enforce
the ship plane-capacity limits. In previous versions, this was enforced only
in the load command, and incorrectly maintained. (In some situations, the #
could go negative, and wrap around to 255, causing much disbelief)
.s1
The telegram command was changed so that bulletins from a deity include the
name of the deity country that sent them. (Useful for multiple deity games)
.s1
The nation files are re-read for the report and country commands. Before,
the program would use the in-memory nation stuff, which meant that it was out
of date if it changed (i.e. people logging in/out after you would not be shown
as logged in in the country output, report would be out of date, esp in blitzes,
etc) With this option, the nation file is closed and re-opened, which updates
the in-memory stuff to be current. This is still not perfect, as when people
exit incorrectly, they are still shown as logged on, until they telnet and
kill the process, or the transaction manager is restarted.
.s1
Also, the init_nats function was changed. Previously, if a country did not
receive a BTU delta when logging in, its last login time was not set.
.s1
.L OPTIONS
.s1
Many new options have been defined for BBN 1.0.
.s1
.L CHOPPER-STEALTH
.s1
Choppers have a chance of evading interception. It isn't as good as
stealth, which is why it's listed as half-stealth. I'm not telling
the exact numbers... figure them out for yourself.
.s1
.L BABY_NUKES
.s1
Credit for this idea goes to Tom Tedrick. Damage/effects are the same,
but blast effects are confined to the target hex. This also automatically turns
of MISSING MISSILES. Note that launch will still ask about air/ground burst.
.s1
.L SUPER_BARS
.s1
Gold bars cannot be pin bombed, and are not hurt by firing. There are still
ways to kill them, but not by shelling or bombing.
.s1
.L EASY_BRIDGES
.s1
A bridge can be built in any sector adjacent to one of yours, providing
that the target sector is adjacent to a non-sea, non-bridge sector. Since
bridgeheads are not required, you can't collapse bridges by killing the head.
(You can still kill them by taking the bridge below 20%) This also fixes a 
problem with planes on bridges not dying when the bridge dies. (This was partly
fixed before, but the fix didn't work all the time)
.s1
Note that the sector building the bridge may be a bridge, providing the sector
that the bridge is being built in is adjacent to a non-sea, non-bridge sector.
For example, this is legal:

 . . . . . . .	=>	 . . . . . . . 
. . . . . . .	=>	. . . . . . . 
 . - = . - . .	=>	 . - = = - . . 
. . . . . . .	=>	. . . . . . . 
 . . . . . . .	=>	 . . . . . . . 

.s1
This is not legal:
 . . . . . . .	=>	 . . . . . . . 
. . . . . . .	=>	. . . . . . . 
 . - = . . - .	=>	 . - = = . - . 
. . . . . . .	=>	. . . . . . . 
 . . . . . . .	=>	 . . . . . . . 

.s1
Also, the sector building the bridge must be at least 60% efficient.
.s1
.L BETTERARMOR
.s1
Ship's armor factors now more greatly affect both shelling and bombing. BB's are
much harder to kill. Smaller ships are more fragile.\
.s1
.L ALL_BLEED
.s1
When this option is enabled, countries receive tech leakage from ALL tech
producers, not merely allies. Tech leakage is 33% by default. The leak is
calculated as 33% of the average tech made by all those making tech. For
example, if there are 10 players, making 3, 4, 0, 0, 3, 0, 2, 0, 2, 1 tech,
the total tech made is 15 by 6 countries, for an average of 2.5 tech. All 10
countries would get .833 tech leak (33% of 2.5), for total tech production of:
3.833, 4.833, 0.833, 0.833, 3.833, 0.833, 2.833, 0.833, 2.833, 1.833
.s1
.L DRNUKE
.s1
You need research to build nukes. (1/4 the tech value, so a tech 300 nuke
needs 100 research)
.s1
.L SLOW_WAR
.s1
You cannot attack someone's land unless you are at war with them. (You can
still blow up their ships with fire, board, torp, etc) When you declare war on
someone, your relation goes to Mobilizing. You still can't attack them. After
the next update, your relation goes to Sitzkrieg, and you still can't attack
them. After the NEXT update, you're at war with them, and anything goes.
.s1
If someone is Mobilizing or in Sitzkrieg, or At War with you, and you
declare war against them, your relation is set to be the same as theirs, so
there is no way to gain any advantage through this stuff. It just means that
you can't sneak-attack people.
.s1
The restrictions do not apply if you are the old_owner of the sector. If you
are the old_owner, you may bomb/shell/attack at will.
.s1
Note that while ships are freely attackable, regardless of war status, even
if they are in a harbor of a nation you are not at war with.
.s1
The first person who declares war pays $1000. Return declarations are free.
.s1
.L NOMOBCOST
.s1
Ships do not use mobility when firing. Torpedo mobility cost is reduced to be
equal to 1/2 the mobility the firing ship would use to move a sector.
.s1
.L NEWSUBS
.s1
This option encompasses several small changes in sub-warfare. A new flag, sub-t,
is added. Only ships with the sub-t flag are allowed to torpedo subs. Also,
ships with depth-charge capability can now use them to their full gun range.
Depth charges are more affected by sub armor value. Torpedo damage is increased.
.s1
.L OTHER-CHANGES
.s1
Most of the ships have had their firing ranges bumped up significantly.

Thomas Ruschak, The_Scum
.s1
CHANGES.CHAINSAW.2.0
.s1
Changes to the Chainsaw code in version 2.0
.s1
This file enumerates some of the more important changes between
the code used for BBNII (Chainsaw Empire 1.0) & Chainsaw 2.0.
.s1
.L Bug-fixes
.s1
You can no longer map by attempting to land planes on unknown sectors.
The replies have been standardized to give no information.
.s1
The radar command is fixed to eliminate 'ghost' images at high tech.
.s1
Planes can now land on carriers that happen to be in bridge sectors.
.s1
You can no longer find out if any enemy subs are under your bridges by
trying to load things on them.
.s1
You can no longer load things onto non-allied ships, even
if they are in a harbor you own.
.s1
'break' now breaks ALL sanctuaries you own, even if some are not adjacent
to 0,0.
.s1
When specifying a flight path, the terminating 'h' is no longer counted as
a sector for purposes of range.
.s1
Qorder is now documented, and works as described in the document.
.s1
.L CHANGES
.s1
When scrapping or scuttling groups of ships/planes or wings of
ships/planes, you will be asked to confirm that you want to do it.
(Also when doing scrap * or scuttle *)
.s1
Ships that assault will always retain 1 food.
.s1
You may now specify a "number to build" parameter for build.
.s1
The deity can now make declaration on another country's behalf.
.s1
Minesweepers now take less damage from mines (1/2 the normal amount).
.s1
A plane unloaded in an ally's harbor now becomes his plane. A plane loaded
onto an ally's ship also becomes his plane.
.s1
Your planes may now land on an ally's sectors or ships. Once there, they become
his planes.
.s1
Ships will fire back in defense of sectors being attacked,
assaulted, or shelled.
.s1
Your ships/sects will no longer shoot themselves in defense.
You can cheerfully blast your own ships/sects without fear of
retaliation from your own forces. You can also torp your own 
ships, and they will resignedly watch as the torps slam into them.
.s1
Gun ranges are no longer truncated. Instead, there is a % chance equal to the
decimal portion of the range of gaining a hex. For example, with a range of
2.50, there is a 50% chance of being able to fire at range 3. With a range of
6.33, there is a 33% chance of being able to fire at range 7.
.s1
The power report has been changed to be more realistic. The relative worth
of most commodities has been changed, and a country's total is multiplied by
a tech factor to arrive at a final value.
.s1
There is no longer a minimum maintenance cost of $1/ETU for planes and ships.
The new minimum is $1.
.s1
The torpedo command is changed. Surface ships may now have the torpedo ability,
as well as subs. When surface ships torpedo, they are fired at by defending
ships BEFORE they fire their torpedos.
.s1
An "escort" flag was added for planes. Planes with the escort flag may escort
missions. (Formerly, only planes with the intercept flag could escort)
.s1
Fighter aircraft being used as tactical bombers are now at a greater penalty
in air-to-air combat.
.s1
Ships with no mobility may now be nav'd. This allows fishing boats and oil
derricks to check resource values without having mob. (Of course, you can't
move with no mob, and an attempt to do so kicks you out)
.s1
The load command will now allow you to load wings/groups of planes.
.s1
You may now give things to allied ships using the tend command. (You may not
take things from allied ships, only give) (If ALLYHARBOR is used)
.s1
You may now upgrade allied ships. (If ALLYHARBOR is used)
.s1
Your civs will work on allied ships in your harbors. (If ALLYHARBOR is used)
.s1
You may assault your own sectors. Mil assaulting are simply added to the 
number in the sector.
.s1
You may now land planes on a carrier in an allied harbor. (If ALLYHARBOR is
used)
.s1
Conventional non-tactical missiles will now be intercepted by ABM's.
.s1
.L Options
.s1
.L Trade-Ships
.s1
Trade ships simulate internal or external trade. In the real world, you make
goods someplace where they're cheap, ship them somewhere they're expensive, and
sell them, making a profit.
.s1
For empire purposes, you make a trade ship, which uses many cm's, and costs
$2500. You then sail it somewhere and scuttle it. You get money back, more than
you put into building the ship. The return rate depends on several factors.
.s1
The primary factor is distance. The distance is calculated as straight-line
distance from the sector the ship is scuttled in to the sector in which it was
built. You have to sail it at least 8 sectors to get any profit at all. If you
sail it more than eight, the profit rises steadily.
.nf
		dist < 14:	+3.5% * dist
		dist < 25:	+5.5% * dist
		otherwise:	+7.5% * dist
.fi
.s1
Another factor is the sector in which you scuttle it. If you scuttle it in a
harbor you own, you get normal cash. If you scuttle it in the harbor of
an ally, you get a 20% bonus, AND your ally gets 10% of the cash you make as
a bonus (you still get the full amount)
.s1
For example, using the numbers from above, if you sailed 15 sectors, and
scuttled in an allied harbor, you'd get $4562 * 1.2 = $5.5K, and your ally
would get $4562/10 = $456.
.s1
The last factor is the efficiency of the trade ship. The cash return is
multiplied by the efficiency of the ship when it is scuttled.
.s1
Note that captured trade ships can be scuttled by pirates, who receive cash
as if it was their own.
.s1
.L FUEL
.s1
Each ship has a fuel capacity, and a fuel usage rate. During an update, ships
use fuel at a rate of 10 mob for every <fuel-usage> units of fuel. Ships with
a fuel usage of 1 get 10 mob for every 1 fuel, ships with 2 fuel usage get 10
mob for 2 fuel, etc. Ships with no fuel do not get mobility. Ships with a fuel
usage of 0 do not need fuel. (Sailing ships & nuc ships/subs)
.s1
A new command, "fuel" is added. It allows fueling from a harbor sector, or a 
ship or ships with the "oiler" flag. When fueling, petrol and/or oil in the
harbor or on the oiler is exchanged for fuel points, which are loaded on the
ship. Petrol is used first, if available, at a rate of 1 pet = 10 fuel. If
there is not enough petrol, oil is used, at a rate of 1 oil = 50 fuel.
.s1
Ships of allies can be fueled, if the ALLYHARBOR option is in use, either at a
harbor, or with an oiler.
.s1
.L SEMILAND 
.s1
This option adds a flag, "semi-lander", which allows the ship to use 25% of its
mil in an assault.
.s1
.L SHIPCHOPPERS
.L XLIGHT
.s1
These two flags are similar. They establish new plane flags, "helo" (ship
helicopter), and "x-light" (extra light). Each ship is then rated on it's
ability to carry those types,
independently of it's ability to carry normal planes. This allows deities
to let non-carriers carry sams, for example, or to let ships carry anti-sub
helicopters or transport helos.
.s1
.L ASW_PLANES
.s1
A new plane flag is added "asw". This allows planes with the flag to spot
subs on recon missions. (Detection chance is
(100-accuracy)-(4-sub visibility)*10)
Planes with this flag may be informed (depending on detection ability) when
pin-bombing a sea-sector of the presence of subs. They then have the option of
bombing the subs. Each plane on such a mission rolls separately to find the
subs, and can only bomb the ones it can find.
.s1
.L PARAFLAG
.s1
A "para" flag is added for planes. Only planes with the para flag can be used
to paratroop.
.s1
.L STEALTHV
.s1
This option adds a new statistic for planes, stealth percentage. Use of this
option effectively disables stealth planes and the CHOPPER_STEALTH option, as
the stealth and half-stealth flags are ignored.
.s1
With this option, each plane is rated as to its percentage chance to evade
ground-fire and interception. The stealth rating of a group of planes is equal
to the rating of the plane in the group with the lowest stealth rating.
.s1
.L MULTIFIRE
.s1
When this option is used, both fire and torpedo accept multiple ships/sectors.
Each firing ship/sector may be used against a separate target, or all may fire
together.
.s1
Return fire is distributed evenly over the firing ships/sectors, with each
defending ship/sector firing once. The spreading of damage also applies to
subs being depth-charged after torping, and for torpedo boats being fired at
prior to torping.
.s1
.L RETREATS
.s1
Each ship/fleet may be given a retreat path, and retreat conditions. Conditions
are: retreat when damaged at all, retreat when torped, retreat when sonared,
retreat when fired upon from beyond max range, retreat if bombed, retreat if
depth-charged.
.s1
Retreat conditions are assigned to ships, and may be assigned to entire fleets
at once. When a ship's retreat condition is satisfied, things happen.
.s1
If the ship's fleet has a retreat path, all ships in the fleet that are in the
affected sector attempt to retreat 2 sectors along the path, using mobility as
if they were navigating.
.s1
If the ship's fleet does not have a retreat path, then the ship checks to see
if it has it's own retreat path. If so, it retreats 2 sectors along it, as
above.
.s1
Note that retreating ships can hit mines, and could be blocked by new bridges,
or by an ally changing to non-allied status (which would make them unable to
retreat into a harbor)
.s1
Retreat paths are wiped from a ship when the ship moves. Retreat paths are wiped
from fleets only at the player's request.
.s1
.L HARBOR_POLICE
.s1
With this option, the board command is extended to allow boarding of enemy
ships in your harbors. The harbor sector must have mobility, but the command
does not use any mobility.
.s1
.L MINE_PLANES
.s1
With this option, planes with the \*Qmine\*U ability are allowed to use
the drop command to drop shells as mines in sea sectors. (see info drop)
.s1
.L SWEEP_PLANES
.s1
With this option, planes with the \*Qsweep\*U ability will automatically
attempt to sweep mines while on recon missions. See info recon for more
details.
.s1
.L BUDGET
This option changes the update ordering a bit, and allows you to set
priorities for production, for those situations where money is tight.
A new command, \*Qbudget\*U, is added, that shows financial information,
and allows setting of priorities.
.s1
.L DAVYJONES
.s1
When this option is used, nukes striking wasteland or sanctuary sectors do
not detonate. Nukes striking water sectors affect only their impact sector.

.s1
Thanks go to all the people who've helped me with ideas and playtesting:
Jorge Diaz (Ansalon), Tom Tedrick (Afrika Korps), Keith Graham (DreamLands),
Dave Nye (Evil_Empire), Sasha Mikheev (Dolgopa), Baldric, Elsinore, Brett Reid
(Resvon), Kazzadur \ystein Tvedten (Flipper's evil twin), Tharkadia, Yikes, and
many more I'm forgetting. The good things are mostly ideas contributed by these
people. The bugs, I'm afraid, are my own.
.nf

					Thomas Ruschak (The_Scum)
.fi
.s1
CHANGES.CHAINSAW.2.1
.s1
Changes to the Chainsaw code in version 2.1
.s1
Chainsaw 2.1 has some minor differences from Chainsaw 2.0.
.s1
Now, when navigating, sonar or radar commands will
default to the current ship, or the first (lowest 
numbered) ship in a group. You may override this by
specifying the desired ship on the command line.
(I.e. "r 12" or "radar 212", etc)
.s1
The chance of a plane aborting is changed to 
(100-eff)%. This means that if a 100% plane
takes a hit for 30% damage, it has a 30% chance to abort.
If it then takes another hit for 20% more, a total of 50% damage,
it has a 50% chance to abort.
.s2
Also, when bombing ships, rolls are not made for
aborting until after all ships have fired, rather
than rolling every time the plane takes a flak
hit from a ship.
.s1
A new command has been added, the 'payoff' command. 
It shows the value of trade ships if scuttled at
the current sect (check info payoff).
.s1
Constants have been added to control ship & plane 
efficiency growth rate. It is now possible to allow
2xETUS growth in efficiency, 3x, etc.
.s1
Subs may no longer shell.. they may only torpedo.
.s1
A WIRE option has been added that activates the
'wire' command. The wire command reads only
announcements. The normal read command then
reads only telegrams & bulletins.
.s1
When assaulting, troops will always leave at least 1 food on the ship.
.s1
You may now build multiples of an item by adding a build count at the 
end of your build command. (see info build)
.s1
You may now give comments in the hours file. Comment lines start with a
'#' character. (Players do not need to care about this.. it's for deities)
.s1
The report command has been changed to not include countries
that are not active. (i.e. visitor or in sanctuary)
.s1
The version command now show the ship/sector/plane mobility max/gain,
as well as the ship/plane growth constants.
.s1
CHANGES.CHAINSAW.2.2
.s1
Changes to the Chainsaw code in version 2.2
.s1
Chainsaw 2.2 has some minor differences from Chainsaw 2.1.
.s1
A new option, SNEAK_ATTACK is included. It can only be used in
conjunction with SLOW_WAR. It allows you to attack a sector of
someone you're not at war with. Once attacked, the attacker 
automatically declares war against the defender, and pays $5K.
At that point, the attacker has to go through the normal SLOW_WAR 
procedure before attacking again. See info attack, info assault.
SNEAK_ATTACKs may not be done by parachute.
.s1
The 'list of commands' command now puts a 'c' in front of commands
that require you to have a capital to use, and a '$' in front of
commands that require you to be non-broke for use.
.s1
CHANGES.CHAINSAW.3.0
.s1
Changes to the Chainsaw code in version 3.0
.s1
Chainsaw 3.0 has some major differences from Chainsaw 2.2.
.s1
Ships that don't carry enough crew lose effic when at sea. See
info build (the ship building section)
.s1
The BUDGET option has been removed. Budget is now standard.
The NEWSUBS option has been removed. NEWSUBS is now standard.
The MULTIFIRE option has been removed. MULTIFIRE is now standard.
.s1
I took out the 'too much traffic for workers' thing, because it was dumb.
.s1
Ships of tech 310 or better can detect mines with sonar.
.s1
Conventional missiles launched from subs are anonymous. The
victim is not notified of the launching country's identity,
and the news knows naught.
.s1
In order to export something from a sector, you must have
military control of the sector. In order to import something from a
distribution sector, you must have military control of the distribution
sector.
.s1
The chances to hit a mine are changed. See info mine and info lmine.
.s1
Two new deity commands have been added, 'disable' and 'enable',
which disable and enable updates. The 'update' command will mention
it if updates are disabled.
.s1
When you capture a sector, all distribution info is wiped (distribution
sector, thresholds).  Sectors just taken over are stopped (see start and stop).
.s1
The criteria for sector ownership have been changed slightly. See
info sector-ownership for information.
.s1
Countries still in sanctuary can now get telegrams and announcements.
.s1
Fishing boats and oil derricks only work in open ocean sectors.
.s1
Civs & mil may be distributed. No sector will export its last civ.
.s1
A new command, bdesignate, has been added. It allows you to manually
modify your bmap.
.s1
When navigating a ship or marching a unit, you get a map of the sectors around
your ship or unit, without using BTU's.
.s1
When using map or bmap, giving the number of a ship you own instead
of an area will give you a map centered around that ship. For example,
.s1
.EX map 1
.s1
will give you a map centered on ship 1. Lmap works similarly, except
you can get a map around a land unit, and pmap gives a map around a plane.
.s1
When loading/unloading, you can give a desired level as a negative number
instead of an amount to load/unload. For example:
.s1
.EX load 4 -10 food
.s1
will attempt to load or unload food as necessary to get ship 4 to 10 food.
(Similar to the syntax for enlist) See info load for details.
.s1
Land units have been added. There are many new commands for dealing with them.
Look at info Land-units for a general overview.
.s1
Sectors that don't have enough food at the update will now attempt to 
draw food from supply sources. (see info supply for details on supply sources.
Conquered sectors will not draw food)
Other sectors/ships/units will now draw supplies (mostly shells) when they
need them.
.s1
You can now give destination sectors instead of paths for all movement
commands. For example,
.s1
.EX move c 0,0 10 5,-1
.s1
will move 10 civs from 0,0 to 5,-1 via the most efficient path, the
one that uses the least mobility.
.s1
Ships/planes/units now use lcms/hcms as they gain efficiency, rather than using
them all at once. See info build for more details.
.s1
Distribution has been changed. Each sector now has a distribution sector
rather than a distribution path (during the update, the least-cost path
is found and used) Also, importing & exporting via distribute now happen
in separate  phases, rather than being mingled. See info distribute 
and info update-sequence for a full description.
Read info distribute, or don't cry.
.s1
There is a document that details the update sequence. (info update-sequence)
.s1
Civs/mil/uw's are much harder to kill (how much harder is deity-settable)
with shelling or bombing or nuking.
.s1
Air to air combat is changed. Check info interception for an explanation
of the interception process and a description of air combat.
.s1
Fully efficient roads now have a small mob cost to move through.
.s1
Fort gun range is now (7*tech factor). Damage from fort shelling now
depends on the number of guns firing. Forts get a +1 range bonus if at
least 60% efficient.
.s1
Sectors can cost money to designate, and may cost extra to build. They
may also take lcms or hcms to build. Show sector build will show this.
.s1
A new command, cede, allows you to give a sector or ship to a neighbor. See
info cede for more details.
.s1
There are several new information commands, such as neweff and 
starvation. See the info pages for details.
.s1
Sectors are damaged by combat occurring in them. See info attack for details.
.s1
Missions may be given to ships/planes/units, allowing them to interdict 
enemy movement, support friendly attacks & defense, and other things.
See info mission for details.
.s1
Forts automatically fire at enemy ships coming within range.
.s1
Mountains can be used (poorly) as capitals. See info capital.
.s1
New options:
.s1
RES_POP: when RES_POP is enabled, the maximum number of civilians, mil,
or uw's that can be in a sector is determined by research. See info
research for exact details.
.s1
NEW_STARVE: when NEW_STARVE is enabled, the starvation ordering is changed.
Instead of losing equal numbers of each type of population (uws, civs, mil),
you first lose uw's, then civs, then mil. As before, no more than 1/2 the total
population of a sector can starve in one update. Example: if you have 200 uw's,
100 mil, and 500 civilians, and no food, normally you'd have 100 uw's, 50 mil,
and 250 civs after the update. With NEW_STARVE in effect, you'd have 0 uw's,
300 civilians, and 100 mil. 
.s1
NEW_WORK: When moving civs to or from a sector with low work, the new loyalty
and work percentage of the destination sector is computed proportionally.
For example, of you move 100 people from a 0% work sector to a 100% work
destination sector with 100 people in it, the destination sector will have 200
50% work people in it. If you then moved in 100 people with a 100% work
percentage, the sector would have 300 67% work people in it.
.s1
CARRIER_WORK: carriers will now work on damaged planes on board. It's not much,
but it's something.
.s1
STOP: there are now 2 new commands, stop & start. If you stop a sector, it
will not gain efficiency or produce for 1 update. Start counteracts stop.
Census is also changed to report the production status of a sector.
.s1
CHANGES.CHAINSAW.3.3
.s1
Changes to the Chainsaw code in version 3.3
.s1
Sectors taken over have the their production stopped.
This should prevent the "redesignate a dozen tech centers" update attack.
If STOP is not defined, the new sector designation is set to be the
same as the current sector designation.

.s1
The 'file' utility now creates an all-sea world instead of not creating a world.
.SA "Server"
