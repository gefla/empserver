.TH Command LBOARD
.NA lboard "Board enemy land unit"
.LV Basic
.SY "lboard [<VICTIM UNIT>] [<FROM SECT>]"
The lboard command enables the owner of a sector to engage in hand-to-hand
combat with a land unit in that sector.
To board unit #5 from sector 1,-1 you might type:
.EX lboard 5
.s1
The program will ask from which sector you wish to board and
how many troops to board with.
.s1
.s1
In order to board a land unit from a sector, the land unit must be in the
sector, and the sector must have mobility.
.s1
Some things to note:
.s1
Any land units in the combat will fight as if they were simply mil,
i.e. they won't get any offensive of defensive multiplier, except for
"marine" units which get their full bonus.  Similarly, units
on land units don't have to take morale checks, as they are fighting as mil.
Mil on the victim unit will still die before units take casualties, however.
.s1
Note that you can only board with as many mil that the
unit you are trying to board can hold.
.s1
See info \*QAttack\*U for the exact procedure.
.s1
.SA "Attack, LandUnits, Combat"
