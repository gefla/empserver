.TH Command MORALE
.NA morale "Change the retreat percentage of a unit"
.LV Expert
.SY "morale <UNITS> <retreat percentage>"
The \*Qmorale\*U command is used to set the retreat percentage of land
units.
.s1
Normally, a land unit must start making morale checks during combat
when its efficiency is less than or equal to its retreat percentage.
(The chance of failing a morale check is: sqrt(100-effic)%/(men in unit).
Example: a 25 man unit is at 75% efficiency. It's chance of failing is
sqrt(100-75)%/25 = sqrt(25)%/25 = 20%)
.s1
The morale command
allows you to adjust this percentage within a range. If, for example, you
wish only to trade space for time, you could set the retreat percentage
to 100%, meaning that your units would retreat often, but would still slow
the enemy down a bit. Or, you could set your retreat percentage to the minimum,
and make the unit stick around and fight more, but at the cost of more
casualties.
.s1
The maximum allowed value is 100% (i.e. start making morale checks after
any casualties) The minimum values is based on happiness:
.ti 3
(morale_base) - happiness
.s1
"morale_base" is a deity settable variable, which defaults to 42.  When
a unit is first built, it's retreat percentage is set to "morale_base".
.s1
.SA "attack, assault, LandUnits"
