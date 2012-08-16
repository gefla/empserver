.TH Concept Damage
.NA Damage "How much damage stuff does"
.LV Expert
.nf
It is possible to be damaged by shells, bombs, land mines, sea mines,
and nuclear blasts.  This info page shows you how much damage each of
these things does.  In the following formulas, random(x) means a
random number from 1 to x.  Once the damage has been calculated, then
it is applied.  Damage applies to different kinds of things in
different ways (see below).


1. Calculating the damage

Shelling
You can be shelled by a fort, artillery unit, ship, depth-charge, or torpedo:
       fort damage = guns * eff * (random(30) + 20) / 7
       unit damage = guns * eff * (5 + random(6))
       ship damage = guns * eff * (10 + random(6))
      depth damage =    3 * eff * (10 + random(6))
       torp damage = 40 + random(40) + random(40)
Note that for forts, a maximum of 7 guns may be fired.

Bombs
A plane drops a number of bombs equal to the "load" the plane can carry.
Each bomb does the following damage:
     blam: 1 + random(6)
     Blam: 5 + random(6)
     BLAM: 8 + random(6)
When pinbombing a ship, plane, or land unit, damage is doubled.
Whether you get a blam, Blam, or a BLAM depends on the accuracy of the
plane and the difficulty of the target (see info bomb).

Land mines
Land mines damage commodities by random(20) and land units by 10 + random(20).
If the land unit is an engineer, then the damags is cut in half.

Sea mines
Sea mines damage ships 21 + random(21).  If the ship can sweep, then
the damage is cut in half.

Nuclear Detonation
Damage from nuclear detonation uses the following formula.  From
"show nuke stats", you see that each nuke has a certain blast radius
and %damage.  If you are groundbursting, then multiply the radius by
2/3.  If you are airbursting, then multiply the radius by 3/2.  Now
based on the distance from gound zero, the nuke does the following
amount of damage:

Groundburst: dam / (dist + 1)
   Airburst: 0.75 * dam - 20 * dist

For the small, medium, and large nukes currently in the game
(damage 80, 90, 101 and radius 1, 2, 3) the following damage is obtained:

         Groundburst          Airburst
         0       1            0       1       2       3
small   80                   60      40
med     90      45           68      48      28
large    /      50           76      56      36      16


2. Applying the damage

Once you know how much damage the bombs or whatever have done, then
you apply the damage to what was hit.  In the calculations below,
%damage will never be larger than 100%.  If it is, then we just say
that the %damage was 100%.  Once we have the %damage, then we apply it
as follows:

new efficiency = (old efficiency) * (1 - %damage / 100)

For example a 50% ship hit for 10% damage would go down to 45%.

The following formulas show you how to get %damage from damage.  Note
that these formulas are used in all cases _except_ nuclear damage.  In
the case of nuclear damage, %damage is always equal to damage (except
for planes hardened in silos which subtract their "hardened" value
directly from the damage).

Ship
%damage = damage / (1 + def / 100)

Land unit
%damage = damage * (vul / 100) * (127 / fortification + 127)

Plane
%damage = damage

Nuke
can't be damaged by conventional weapons
nuclear damage may destroy it (chance in percent equal to damage)

Sector
sectdamage = damage / ((sectdef - 2) * eff + 2)
%damage = (100 * sectdamage / (sectdamage + 100))
commodities take %damage
land units take 0.3 * %damage
planes take 1/7 of the damage done to land units

Commodity
%damage = (100 * damage / (damage + 50))

Note: when a ship, plane, land unit or bridge is destroyed, its
contents is lost.  This includes nukes.

.fi
.SA "fire, launch, torpedo, lmine, bomb, Hitchance, Attacking, Combat"
