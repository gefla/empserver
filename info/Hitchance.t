.TH Concept Hitchance
.NA Hitchance "The chance of a projectile hitting its target"
.LV Expert
This page describes the chance of a mine, torpedo, ship anti-missile
defense, missile, or plane hitting its target.
.nf

Sea Mine hitchance
The chance of a ship hitting a mine at sea is (mines/(mines+20)). (For
example, with 20 mines, the chance of hitting one is 20/40 = 50%) The
damage is dependent upon the size of the ship and the spot that the
mine hits.  (see \*Qinfo Damage\*U).


Land Mine Hitchance
The chance of a land unit hitting a land mine is mines/(mines+35).
(For example, with 20 mines, the chance of hitting one is 20/55 =
36%).  See \*Qinfo Damage\*U to find out how much damage things take
from land mines.  Note that the chance of land units hitting land
mines is divided by 3 if the land units are accompanied by engineers.

When moving commodities (civs, mil, shells, etc) or transporting
planes, the chance is also dependent upon the weight being moved.
First the above chance is checked, and then a chance equal to (weight
/ 100) is checked.


Torpedo hitchance
The chance of a torpedo hitting its target is equal to:
  hitchance = 0.9/(range + 1)
where "range" is the range to the target.  If the visibility of the
ship firing is less than 6, then (5 - visibility) * 0.03 is added to
the hitchance.

Thus, the base chance of a sub hitting its target (before visibility
modification) is:

     Range   Chance
     0       90%
     1       45%
     2       30%
     3       22.5%
     4       18%
     5       15%


Ships anti-missile defense
If a ship has "anti-missile" defense capabiilty, then it will
intercept any marine missiles launched at ships within 1 sector of the
ship.  Only missiles belonging to a country you are "At War" with will
be intercepted.  Only ships which are at least 60% efficient will be
able to use their anti-missile defenses.  The chance of the ship
hitting the incoming missile it equal to:

  hitchance = gun * eff * tfact * 4.5 - (missile "def" value)
where
  gun = the number of guns the ship is allowed to carry
  eff = the efficiency of the shiip
  tfact = tech / (tech + 200)
where
  tech is the tech level that the ship was built at


Plane and missile hitchance
If the plane or missile is carrying a nuclear warhead or the target is
a sector, then the hitchance is 100%.  Otherwise, the following
procedure is used to calculate hitchance.

The formula for a plane or missile hitting its target depends on the
type of the target (ship, plane, land unit) and the "hardtarget" value
of the target (which represents how hard that target is to hit).

Ship:
  vis = the visibility of the ship (from 'show ship stats').
For a sub, we set vis = visibility * 4.
If the ship is at sea, then:
  hardtarget = (eff of ship) * (20 + speed/2 - vis)
otherwise:
  hardtarget = (eff of ship) * (20 - vis)
The hardtarget of a fleet is the hardtarget if the "easiest" target in
the fleet.

Land unit:
  hardtarget = (efficiency) * (10 + (sector defense) * 2 + speed/2 - vis)
The hardtarget of an army is the hardtarget of the "easiest" target in
the army.

Plane:
If the plane is in the air (satellite, icbm), then
  hardtarget = the "def" value of the plane.
otherwise if the plane is on the ground:
  hardtarget = 0

Once we have calculated the "hardtarget" value of the target, then the
chance of a plane or missile hitting its target is equal to:

  hitchance = acc - hardtarget

where
  acc = (efficiency) * (1 - 0.1 * tfact) * (1 - placc/100)
where
  tfact = (pltech - mintech)/(pltech - mintech/2)
where
  pltech = tech level of plane
  mintech = min tech level required to build plane
and
  placc = plane accuracy
and if the target is a ship and the plane is an ASW plane, then 
  placc = placc - 20
and if the target is a ship and the plane does not have tactical capability
  placc = placc + 35

Lastly we smooth out the bottom end of the hitchance curve.  If
hitchance is less than 20, then it is "belled up" using the following
curve:
  hitchance = 5 + 300 / (40 - hitchance).

On the other hand, you can just find out the hitchance by pinbombing
or launching a missile--the hitchance is always printed!  :-)
.fi
.SA "fire, launch, bomb, torpedo, Attacking, Damage, Interception, Combat"
