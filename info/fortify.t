.TH Command FORTIFY
.NA fortify "Fortify a land unit"
.LV Basic
.SY "fortify <UNITS> <MOBILITY>"
The \*Qfortify\*U command is used to make units dig in and
make themselves harder to damage. The maximum amount of
fortification is the same as the maximum amount of mobility
a unit can have. (The version command will print this info).
Each point of mobility used increases the fortification value by one.
.s1
If <MOBILITY> is a positive number, at most that much mobility will be
used.
.s1
If <MOBILITY> is a negative number, at least -<MOBILITY> will be left
in each unit.
.s1
A fully fortified unit is twice as hard to hurt as it normally
would be, and is twice as strong on the defense.
.s1
Units fortifying in a sector containing an engineering unit
are better at it: they get 50% more fortification per mobility.
.s1
If a unit moves or retreats, it loses all fortification value.
A unit that reacts to defend a sector, on the other hand, does NOT
lose its fortification unless it is forced to retreat. It receives
the fortification bonus in combat, and is still fortified when it
returns to its original sector after the battle.
.s1
Note that fortification will not help a unit fight Guerrillas in
an occupied sector (see info Guerrilla).  Also note that fortifying
a unit does not affect it's mission status.
.s1
Units with maximum mobility automatically use further mobility gains
to fortify.  For example, when a unit with 120 of 127 mobility gains
60 mobility, its mobility increases to the maximum 127, and it uses 53
mobility to fortify.
.s1
Units requiring fuel (if option FUEL is enabled), do not fortify
automatically, as this would burn fuel.
.s1
.SA "bomb, Unit-types, LandUnits"
