.TH Concept "How to take a sector or ship from the enemy"
.NA Attacking "Details for attack, assault, paradrop, board and lboard"
.LV Expert

This info page describes 5 different commands: "attack", "assault",
"paradrop", "board" and "lboard".  These are the five commands that you
use to take something (either a ship, land unit or a sector) from the
enemy by force.

Note that much of the board information relates to the lboard information,
just apply it to a land unit instead of a ship.

.NF
A combat has 15 steps:
(1)  Ask the aggressor for the target sector/ship.
(2)  Ask the aggressor further details about the offensive force.
(3)  Compute initial strengths.
(4)  Move reacting defensive units to the target sector.
(5)  Calculate defensive support.
(6)  Fight.
(7)  Spread the plague.
(8)  Take mobility and supply from land units.
(9)  Send reacting units home.
(10) If successful, the aggressor takes the target sector/ship.
(11) Remove mobility and efficiency from the target sector/ship.
(12) Remove mobility from the aggressor's sectors/ship.
(13) Possibly ask the aggressor for mil and units to move into the target.
(14) Possibly interdict the aggressor's mil and units as they move.
(15) Charge the aggressor BTU's.
.FI
.s1
.L "(1)  Ask the aggressor for the target sector/ship."
.NF
Sea, sanctuary, and wasteland are illegal targets.
You may not assault a mountain.
You may not paradrop into a mountain, capital, city, or fortress.
You may only board a ship from the sector the ship is in, you may
not board a ship which is faster than your ship, and you may not board
a submarine at sea.
You may only board a land unit from the sector the land unit is in.
.FI
.s1
The "assault" command is the only command which will let you attack
your own sector (you can use this to get mil & assault units on
shore).  You should not "attack" unowned land (see info explore).
.s1
If the SLOW_WAR option is enabled, you will not be able to attack
a sector owned by a country you are not AT_WAR with, unless you
are the old owner of the sector.
.s1
.L "(2)  Ask the aggressor further details about the offensive force."
.s1
When you board, you are asked for a ship or sector to board from.
If you are attacking or assaulting, then you have the option to
specify whether you want your forts/ships/plane/units to support your
attack.  If no support is specified, then it is assumed that
you want support from all sources.
.s1
If you are not paradropping, then you will be asked for mil and units
you would like to attack with.
You must always leave at least 1 mil in
the sector or ship you are attacking from, so you may never attack
with all of your mil out of a sector or ship.  
The following limitations apply to the aggressor's mil:
.NF
Attack:  You are limited by the mobility of the attacking sector
         according to the mob-cost to move the mil into the target sector.
Assault: If the target sector contains mil, then you may only attack with
         1/10th of your mil on board, unless your ship has "semi-land"
         capability in which case it is 1/4th, -or- if your ship has 
         "land" capability in which case there is no limitation.
Board:   When boarding from a sector, the sector must have mob.  When
         boarding from either a ship or a sector, the number of mil
         you may board with is limited to the maximum number of mil that the
         target ship can hold.  Note that you can board land units
         from a sector only.

The following restrictions apply to the aggressor's land units:
A land unit must have mobility and be in supply (see info supply) in
order to be able to enter the combat.  Land units with "supply"
capability may not attack.  Only land units with "assault"
capability may assault or board.
Attack:  The land unit will be charged as much mobility as it would
         spend marching into the sector.  If that is at least as much
         as for a path cost of 1.0 (typically mountains only), it must
         have that much mobility, else positive mobility is
         sufficient.
Board:   You may only board with as many land units as the target ship
         can hold.  You cannot board a land unit with other land units.
.FI
.s1
When asked whether you'd like to include a certain land unit in the
combat, you will be given a prompt ending in [ynYNq?].  At this
prompt, you can type:
.NF
y - yes this unit
n - no this unit
Y - yes all units in this army
N - no all units in this army
q - quit attack
? - print this help message
.FI
.s1
.L "(3)  Compute initial strengths."
.NF
The offense strength of your land units is mil * off where "mil" is the
number of mil in the unit, and "off" is the offense multiplier of the
unit (see show land stats), with the following exceptions:
Assault: If the unit doesn't have "marine" capability, then its
         offense strength is cut in half.
Board:   If the unit doesn't have "marine" capability, then its
         offense strength is equal to half of the number of mil in the unit.
.FI
.s1
The total offense strength is the number of mil plus the offense
strength of all offensive units.  This number is then multiplied by
the offense value of the sector the mil and units are attacking from
(see show sector stats).  Ships have an offense value of 1.
.s1
The defense strength of a land unit is the total number of mil in the
unit times its defense multiplier (see show land stats).  The only
exception to this is non-marine land units on ships which have a
strength equal to the total number of mil in them.  The defense
strength of units out of supply is cut in half.  The defense strength
of fortified units is multiplied by (127 + fortification) / 127.
.s1
The total defense strength is the number of mil plus the defense
strength of all defensive units.  This number is then multiplied by
the defense value of the sector the mil and units are in (see show
sector stats).  Ships have a defense value of 1 + def/100 (see show
ship stats).  You can use the "strength" command to see the defense
strength of your sectors.
.s1
.L "(4)  Move reacting defensive units to the target sector."
.s1
If the target is not a mountain, then defensive units in nearby
sectors may react.  Defensive units will keep reacting until the
defense strength is 1.2 times the offense strength.  The following
restrictions apply to reacting land units:
.NF
- The efficiency of the unit must be higher than it's retreat percentage.
- The unit must be in supply.
- The unit must have enough mobility to get to the target.
- The unit must be in range.
.FI
If the unit is in a 60% efficient headquarters, then 1 is added to its
reaction radius.  If it is on "reserve" mission, then 2 is added to
the radius.  Note that you can limit the reaction radius of your land
units using the "lrange" command.
.s1
.L "(5)  Calculate support."
.s1
For attack and assault, both offensive and defensive support is
calculated.  For paradrop, only defensive support is calculated, and
for board and lboard there is no support (however, defending ships within range
will fire on the boarding ship or sector before the fight if boarding
a ship).
.s1
The aggressor's support is calculated based on what the aggressor
asked for.  Defensive support is only called in to the extent that it
would be useful.  If the aggressor outnumbers the defender 10-to-1,
then the defender will not have any support.  Otherwise, the defender
will keep calling in support until its strength is at least 1.2 times
the offense strength.
.s1
Fort, ship, and unit support is simply those forts, ships, and units
in range that can fire (see info fire).  Plane support comes from
those planes on "support" mission.  The support multiplier is
.NF
1.0 + fortdamage/100 + shipdamage/100 + planedamage/100 + unitdamage/100.
.FI
.s1
Ships/forts/units belonging to allies of the combatants will support,
if they are at war with the other combatant. (For example, if A is allied
with B, and at war with C, and A attacks C, things belonging to B will
support the attack if able. The same is true for defense. If C attacks
A, the B stuff will help defend A)
.s1
After these four supports are calculated, then land mine support is
added to the defense support.  Defending land mines add (number of mines) *
0.02 to a maximum of 0.40.  If there are attacking engineers present,
this number is cut in half.
.s1
.L "(6)  Fight."
.s1
Total offense and defense strength are multiplied by their support
multipliers to obtain the final combat strengths.  From this, odds are
calculated.  Then in each "round", a chance according to the combat
odds determines whether an aggressor or defender troop is lost.  Troops
killed in combat are first taken from the mil (from the sector
containing the most mil), and then from the units (from the unit
containing the most mil).  Combat odds are recalculated and the
process continues until there is a victor.  Each time there is a
casualty, a character is printed:
.NF
! means a defending soldier bit it
@ means one of your soldiers went doan
.FI
.s1
If the effiency of a land unit is less than its retreat percentage when
it gets hit, then it will need to make a morale check.
The chance of failing a morale check is (retreat
percentage - efficiency).  If the unit has nowhere to retreat to, then
it takes "extra losses" which amounts to an extra 10% docked from its
efficiency.  Otherwise, the land unit retreats to the adjacent sector
containing the maximum number of civs.  Units will never retreat into
mountains.
.s1
Also note that once all of the military on a land unit are killed, the
land unit stops defending.  What this means is that a land unit can
become "trapped" in an sector if an enemy takes it over after killing
all the military on a land unit while the land unit may not be destroyed.
.s1
When planning an attack remember that overwhelming forces greatly increases
your odds; i.e., attacking 10 men with 40 will result in your losing
fewer troops than if you had attacked with 20.
.s1
.L "(7)  Spread the plague."
.s1
If any combatant in a battle is infected with the plague, all potential
comnbatants in the battle are infected as well (if they do not already
have some level of plague already.)
.s1
.L "(8)  Take mobility and supply from land units."
.s1
The "aggresor loss factor" is equal to the number of aggressor
casualties divided by the total number of aggressor troops that went into
the conflict.  Similarly, the "defender loss factor" is calculated.
Then each land unit loses mobility equal to 10 * (loss factor).
Defensive land units on reserve mission only lose half of that.  Also
there is a chance equal to the loss factor that the land unit will use
up some of its supply (see info supply).
.s1
.L "(9)  Send reacting units home."
.s1
If the defending land unit did not retreat, then send it back to where
it came from at no mobility charge.
.s1
.L "(10) If successful, the aggressor takes the target sector/ship."
.s1
Either 1 aggressor mil (first choice) or one land unit (second choice)
is automatically moved into the conquered ship or sector to occupy it.
Since this mil or unit is effectively "chasing out" the remnants of
the defending forces, the defender will not interdict it (because he
won't want to damage his forces with "friendly fire").
.NF
This is what happens to the sector when you take it:
- All delivery and distribution information is wiped.
- The production in the sector is stopped (see info stop).
- Planes in the sector owned by the player you just took the sector from
  lose 30 plus random(100) efficiency and go to 0 mob. If the new efficiency
  is less than 10%, the plane is blown up.
- Units in the sector owned by the player you just took the sector from
  lose 30 plus random(100) efficiency and go to 0 mob. If the new efficiency
  is less than 10%, the unit is blown up.
- The avail is set to 0.
- New che are created (see info Guerrilla).
- Set the loyalty of the sector to 50 (see info Citizens).
- Reduce the mobility to 0.
- The civs in the sector become "conquered populace" (see info Occupation).

When you take a ship:
- Mission, retreat info and fleet info is wiped.
- Planes and land units on the ship are reduced to 10%.

When you take a land unit:
- Mission, retreat info and army info is wiped.
- Planes and land units on the unit are reduced to 10%.

When you take someone's capital, the defender loses half of their
money or $3000 whichever is greater.  If the defender wasn't broke to
begin with, then the aggressor gets half of their money times (1/5 +
4/5 * efficiency of the sector).
.FI
.s1
.L "(11) Remove mobility and efficiency from the target sector/ship."
.s1
The efficiency of the target sector or ship is damaged by a percentage
equal to the total number of casualties divided by ten.
.s1
If the target is a sector, then it will lose a percentage of the
mobility equal to the number of defender mil casualties divided by the
total number of mil that were originally in that sector, up to a
maximum of 20 mobility.  So if for example, your casualty fraction was
70/100, and your sector started with 50 mobility, then you would lose
min(20, 50*70/100) = min(20, 35) = 20 mobility.
.s1
.L "(12) Remove mobility from the aggressor's sectors and ships."
.s1
The pre-combat mobility costs are:
.NF
Attack:  Remove mobility equal to the mob cost to move the mil into the
         target sector.
Board:   If boarding from a ship, the cost is the half of the speed of the
         defending ship times the efficiency of the defending ship.
         If boarding from a sector, the cost is the number of boarding
         mil divided by 5. This applies for boarding of ships and land units.
.FI
.s1
Only sectors are charged an additional post-combat mobility cost.
In this case, the amount of mobility the sector loses is calculated
using the same formula as is used for the defending sector's mobility cost.
.s1
.L "(13) Possibly ask the aggressor for mil and units to move into the target."
.s1
If the combat was an assault, paradrop, board or lboard, then all victorious
mil and units are automatically moved into the target.  The mil are
moved in with no mobility charge.  If the combat is assault, then
the mil will take an amount of food with them proportional to the
number of people leaving the ship.  The mobility cost to land units is
as follows:
.NF
Assault: If the land units are attacking from a ship with "land" capability,
         then land units are charged (update mob) mobility, except for
         "marine" units which are only charged half of that.  For all
         other kinds of ships, land units go to -(update mob), except for
         "marine" units which go to zero.  Here, (update mob) refers to
         the amount of mobility that units gain at the update.
Board:   Marine units are charged 10 mobility and other land units are
         charged 40.     
.FI
.s1
In the case of attack, the aggressor is asked what they'd
like to move in as follows:  First, the aggressor is asked how many mil
they would like to move in from each attacking sector.  This number is
limited by the amount of mobility left in the attacking sector.  Then
the aggressor is asked which land units they'd like to move into the
sector with the same [ynYNq?] prompt as above, the only difference
being that 'q' now means "don't move anything in".  The mobility costs
here are the same as for "move" and "march".
.s1
.L "(14) Possibly interdict the aggressor's mil and units as they move."
.s1
Post-attack interdiction only occurrs in the "attack" command (and not
in "assault", "paradrop", "board" or "lboard").
When you move your mil into the target sector after an attack, you
will risk the same chances of interdiction (number of mil / 200) and
stepping on land mines (see "info lmine") as
if you were moving them using the "move" command.  
.s1
Similarly, once you have specified which land units you'd like to move
into the conquered sector, then all of your units will move in at
once, and they will be interdicted in the same way as though they were
being moved using the "march" command, including the danger of
stepping on land mines.  Note that there is no
post-attack interdiction and no danger of stepping on land mines if the
INTERDICT_ATT option is disabled (see version).
.s1
.L "(15) Charge the aggressor BTU's."
.s1
The BTU cost is equal to 0.5 + (the total number of casualties) * 0.015.
.s1
.SA "attack, assault, paradrop, board, lboard, Combat"
