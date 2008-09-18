.TH Command BOARD
.NA board "Board enemy ship"
.LV Basic
.SY "board [<VICTIM SHIP>] [<FROM SHIP> | <FROM SECT>]"
The board command enables two ships' crews to engage in hand-to-hand
combat.
To board ship #16 from ship #93 you might type:
.EX board 16
.s1
The program will ask from which ship or sector you wish to board,
(you may only board from one ship at a time), how many troops to board
with, and which land units (having "assault" capability) to board with.
.s1
In order to be able to board a ship from a ship, your ship must be as fast
or faster than the defending ship.
If you can't catch him, you can't board him!
.s1
The attacking ship will use, based upon its speed, an amount of mobility
to overtake and board the target ship.
If the attacking ship has no mobility, it cannot catch anybody!
Ships' speed depends upon the speed of the ship, the technology
of the owner, and the current efficiency of the ship.
A 50% damaged ship moves one-half the speed it does when undamaged.
.s1
In order to board a ship from a sector, the ship must be in the
sector, and either the sector must have mobility or it must contain
some land units with the "assault" capability.
.s1
Some things to note:
.s1
The defense strength is multiplied by 1 + def/100 where def is the
defense strength of the ship (see show ship stats).
.s1
Other ships belonging to the victim will fire on you when you
try to board, (if you're within range).
.s1
Any land units in the combat will fight as if they were simply mil,
i.e. they won't get any offensive of defensive multiplier, except for
"marine" units which get their full bonus.  Similarly, units
on ships don't have to take morale checks, as they are fighting as mil.
Mil on the victim ship will still die before units take casualties, however.
.s1
Note that you can only board with as many mil and land units that the
ship you are trying to board can hold.
.s1
See info \*QAttacking\*U for the exact procedure.
.s1
.SA "Attacking, LandUnits, Ships, Combat"
