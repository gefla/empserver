.TH Command ARMY
.NA army "Designate members of an \*Qarmy\*U"
.LV Expert
.SY "army <ARMY> <UNIT/ARMY>"
The army command is used to specify the army groupings
of your units.
.s1
.EX army <ARMY> <UNIT/ARMY>
In the syntax <ARMY> is the alphabetic character to be used as the
army designation.
This character can be chosen from the set of
upper or lower case a-z and tilde (~).
The pseudo-army specification tilde
specifies all units not currently in any army.
.s1
The specification of units, <UNITS/ARMY>,
can have one of several syntaxes:
.NF
example     meaning
-------     -------
23          unit 23
2/14/23     units 2, 14, and 23
c           all units currently in army `c'
~           all units currently in the \*Qnull\*U army
2,3         all units in sector 2,3
-1:3,0:2    all units in the square area bounded by -1,0 & 3,2
.FI
All armies, (with the exception of the `~' army),
are limited to some maximum size
and you will be informed how many units can be added
when this command is run.
.s1
Having units organized into armies can be very helpful in
loading, moving, etc.,
in that fewer commands are required to perform these commands
on groups of units if they can be specified by army number.
.s1
Note that you can remove units from a army by adding them to
the `~' army.  e.g.
.EX army ~ A
This command would purge all units from army `A'.
.s1
.SA "load, lookout, navigate, radar, land, unload, LandUnits"
