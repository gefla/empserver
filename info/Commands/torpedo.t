.TH Command TORPEDO
.NA torpedo "Slip a torpedo to some poor sucker (from a sub or ship)"
.LV Basic
.SY "torpedo <submarine-SHIPS> <target-SHIP>"
The torpedo command is used to shoot other naval vessels
in a sneaky manner.
.s1
The <target-SHIP> argument is the victim ship number and
the <submarine-SHIPS> argument is the number(s) of your submarine or other
ship(s) with torpedo ability.
The program will ask for these if not provided on the command line.
If using the multi-fire option, the program will prompt for targets for
EACH firing torpedo ship, if no target was given on the command line.
.s1
Your <submarine-SHIPS> must meet the following criteria:
.nf
.in +0.3i
1) It must have torpedo ability. (see info show)
2) It must have at least 1 gun (torpedo tube)
3) It must have at least 3 shells (3 shells = 1 torpedo)
4) It must be at least 60% efficient
5) It must have some mobility
.in -0.3i
.fi
Meeting all these criteria, your torpedo will be launched
toward the victim ship and a countdown of seconds
until the expected detonation will commence.
.s1
The range of a torpedo depends on the type of sub, and its tech
level. Currently the range formula looks like this:
.nf
	(frng) * (tech+50)/(tech+200)

	[Note: frng comes from show ship capability]
.fi
.s1
If the target is out of range, you will be told so,
(after your torp is in the water).
See \*Qinfo Hitchance\*U for the chance of a torpedo hitting its target.
.s1
If the target ship is not in a direct line of sight
but in range, the torpedo
will slam into land and you will be notified.  Note that the victim ship
will still see the torpedo being launched and can return a depth charge
attack (see below.)
.s1
A hit will be reported to you as \*QBOOM!\*U,
See \*Qinfo Damage\*U for the damage that a torpedo does.
.s1
In order to simulate the \*Qreal-time\*U situation,
your mobility will be decremented by mobility equal to 1/2 the cost
the torpedo-ship would pay to move a sector for each torpedo fired;
if it goes to zero (or beyond) you will be forced to stick around.
If your torpedo scores a hit,
your victim will be informed of the fact and will be told
the number of the torpedo-ship, but not the country (so you can deny it).
.s1
The victim's only automatic protection against sub attacks is that
any ship of his/hers that is near you and has depth-charge ability
and has shells will drop them on you as depth charges.
Each depth charge does 10% to 40% damage to your sub, depending
on the armor of the sub, and the range of the destroyer.
.s1
The range a ship can depth charge at is the same as its gun range.
.s1
Against surface ships launching torpedo attacks, the victim gets to fire
all his ships & sectors within range at the attackers BEFORE the attackers
get to fire. Damage done by all defending ships is divided evenly over all
attacking ships.
.s1
.SA "attack, assault, fire, Combat, Ships"
