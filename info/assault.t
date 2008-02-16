.TH Command ASSAULT
.NA assault "Attack coastal sector from ship"
.LV Basic
.SY "assault <SECT> <SHIP> [<forts?> <ships?> <arty?> <planes?>]"
.s1
The assault command allows your marines to \*Qhit the beaches\*U.
It is also the only way to get ashore other than by unloading at a harbor,
thus it is the first step to gaining land on a new continent.
.s1
Any ship with military can assault a sector containing 0 mil, however
only a landing ship can assault a sector with military in it with
its full complement of troops.
The other ships will only be able to land a fraction of their troops
in any given assault (usually 1/10th).
This represents the use of the Admiral's gig, the Captain's barge, etc.
.s1
Note that some ships will have the
semi-lander flag, which allows them to land 25% of their troops.
.s1
If you have land units with the 'assault' ability 
aboard the ship assaulting, you will be asked if
you wish them to join in the assault as well.
Assaulting units pay one update's worth of mobility, except for marine
units, which pay only half of that.  Unless assaulting from a ship
with landing capability, mobility is further decreased to one update's
worth of mobility negated (so that the unit will have zero mobility
after the update), except for marine units, whose mobility is
decreased to zero instead.
.s1
.L NOTE
This mobility loss for units happens whether or not you
own the sector being assaulted. If you wish to land troops
without suffering this penalty, make/capture a harbor!
.s1
If you have ship #28 bearing 10 military at 31,53
it can assault land at any of the adjacent sectors,
(32,52; 33,53; 32,54; 30,54; 29,53; or 30,52).

For example:
.EX assault 32,54 28
.s1
This command asks to assault sector 32,54 from ship #28.
If you assault with 9 troops and assuming the sector has no military,
and since there were 10 troops on the ship to begin with,
90% of the food will be taken along by the assaulting troops,
(and each one that makes it will contribute his or her 10%
to the conquered sector). A minimum of 1 food will be left
on the ship. Land units will not take any food from
the ship, and will only have the food they are carrying.
.s1
You will be asked for any information which was
not included on the command line,
(sector to be assaulted, which ship is doing the assaulting,
number of brave troops to disembark, land units to use).
.s1
The offensive value of assaulting units is cut in half to represent
the difficulty of getting to shore, except for "marine" units which
assault at full strength.
.s1
Units may not retreat from an assault, and will take extra casualties
instead if they fail a morale check.
.s1
Other than these factors, assault is the same as normal land combat, with
forts/units/ships/planes firing on both sides to support, etc. Support
arguments are also as in attacking.
.s1
See info \*QAttacking\*U for the exact procedure. The sector being assaulted
will have exactly the same reaction to being conquered as if it were attacked
by land.
.s1
You may assault your own sectors. In this case, the assaulting mil
are added to the mil in the sector, and assaulting land units simply
come ashore.
.s1
You may also use assault to sneak spies on land.  To do this, you must
assault with spies only, and they will try and sneak on land. See "info Spies"
for more information.
.s1
.SA "Spies, Attacking, LandUnits, Ships, Combat"
