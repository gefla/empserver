.TH Command SHOOT
.NA shoot "Dispose of enemy civilians or uncompensated workers"
.LV Basic
.SY "shoot c <SECTS> <NUMBER>"
.SY "shoot u <SECTS> <NUMBER>"
The shoot command allows you to shoot recalcitrant civilans or
uncompensated workers.
In order to be able to keep the populace under control
(i.e., not have them rebel),
you must have one military for every ten civilians.
Any land units in the sector count towards the mil necessary for control.
.s1
Each civ or uw shot uses mobility (see \*Qinfo Mobility\*U for the
exact formula).
.s1
Fractions are rounded randomly up or down, depending on size. For example,
a 0.8 cost has an 80% chance of being 1, and a 20% chance of being 0.
.s1
<NUMBER> is a positive number
that is the number of people shot per sector.
.s1
After the program has collected any necessary information
it will list each sector where your troops shoot down the poor
helpless people along with the number of people murdered.
.s1
If all the civilians in the sector are removed,
and it was a conquered sector,
it will become completely controlled by you,
and you will be notified of this fact.
.s1
Having security troops in the sector will make the firing squads use
less mobility (they're enthusiastic).
.s1
.SA "convert, Mobility, Citizens, Populace, Occupation"
