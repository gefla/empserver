.TH Command MINE
.NA mine "Drop sea mines from a ship"
.LV Basic
.SY "mine <SHIP> <NUMBER>"
This command drops mines in the sea and is functional
from ships with mine-laying capability (see \*Qinfo Ship-types\*U).
.s1
See \*Qinfo Hitchance\*U for the chance of a ship hitting a mine.
.s1
In the command syntax
<SHIP> is the number of one of your ships and <NUMBER> is
the number of mines you wish to drop.
That number of shells, (assuming you've got them),
will be dropped in the sector that ship occupies,
magically becoming mines as they hit the water.
.s1
Mines can only be removed safely by a minesweeper.
(Other ships can remove them by hitting them, but...)
Planes with the 'sweep'
ability can clear mines by flying recon missions through
mined sectors, with each passage through the sector possibly
removing some mines.
.s1
Ships of tech 310 or higher can detect mines with their sonar.
See \*Qinfo sonar\*U for more information.
.s1
Harbors and sea sectors under bridge spans can be mined.
Your ships will automatically avoid these mines when passing through,
(and your minesweepers will not not pick them up).
.s1
Note that the sector is checked for possible mine hits
only when a ship moves into it;
thus, a ship may safely drop mines in a sector and then move out.
On the other hand, the mines know no allegiance,
so moving back into a mined sector is foolish at best
unless it is a harbor or under a bridge span.
.s1
Mines in the same sector as a bridge that collapses are cleared.
.s1
.SA "navigate, Hitchance, Damage, Ships"
