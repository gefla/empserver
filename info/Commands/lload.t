.TH Command LLOAD
.NA lload "Load things onto land units"
.LV Basic
.SY "lload <COMMODITY> <UNITS> <AMOUNT>"
.SY "lload plane <UNITS> <PLANES>"
.SY "lload land <UNITS> <UNITS>"
The load command carries out the loading of units.
Commodities (like guns, civs, food) can be loaded onto any unit capable
of carrying them (to see what cargos a unit can carry, use the show command). 
.s1
Some land units can carry extra-light
planes (typically these are SAM's)
.s1
Some land units can carry other units
.s1
.SY "lload <COMMODITY> <UNITS> <AMOUNT>"
In the first form,
the amount argument specifies the amount of the given commodity
you wish to load on each unit specified.
Amounts greater than that allowed will be truncated to the
maximum possible. Amounts less than 0 indicate that you want to
make the unit/army have that amount. If the unit/army has less
than that amount, more will be loaded. If the unit/army has more
than that amount, the commodity will be unloaded. For example,
.s1
.EX load food 3 -100
.s1
will cause unit 3 to load or unload food so that it has 100.
If it had 120 food, 20 would be unloaded. If it had 50 food, 
50 would be loaded.
.s1
.SY "lload plane <UNITS> <PLANES>"
In the second form,
where you say \*Qplane\*U instead of say \*Qciv\*U a plane/missile
is loaded onto a unit.
.s1
See the \*Qshow\*U command for a table of the maximum amounts of
each items that can be carried on each unit type.
.s1
Note that if either <UNITS> or <PLANES> does not start with a
number (e.g. it is of the form "a" or "*" or "#1" and not of the form
123 or 12,14) then you will not be given any error messages.  This is
so that you can type something like "lload plane * *" without getting a
hundred error messages.
.s1
.SA "lunload, Unit-types, Plane-types, LandUnits"
