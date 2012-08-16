.TH Command MORALE
.NA morale "Change the retreat percentage of a unit"
.LV Expert
.SY "morale <UNITS> <retreat percentage>"
The \*Qmorale\*U command is used to set the retreat percentage of land
units.
.s1
Whenever a land unit takes a hit in combat, it may break off combat
and attempt to retreat.  This is called \*Qfailing a morale check\*U.
The chance for it to happen is unit's retreat percentage - efficiency.
.s1
The morale command
allows you to adjust this percentage within a range. If, for example, you
wish only to trade space for time, you could set the retreat percentage
to 100%, meaning that your units would retreat often, but would still slow
the enemy down a bit. Or, you could set your retreat percentage to the minimum,
and make the unit stick around and fight more, but at the cost of more
casualties.
.s1
The maximum allowed value is 100%.  The minimum value is based on
happiness:
.ti 3
(morale_base) - happiness
.s1
"morale_base" is a deity settable variable, which defaults to 42.  When
a unit is first built, its retreat percentage is set to "morale_base".
.s1
.SA "attack, assault, LandUnits"
