.TH Command FLEETADD
.NA fleetadd "Designate members of a \*Qfleet\*U"
.LV Expert
.SY "fleetadd <FLEET> <SHIP/FLEET>"
The fleetadd command is used to specify the fleet groupings
of your ships.
.s1
.EX fleetadd <FLEET> <SHIP/FLEET>
In the syntax <FLEET> is the alphabetic character to be used as the
fleet designation.
This character can be chosen from the set of
upper or lower case a-z and tilde (~).
The pseudo-fleet specification tilde
specifies all ships not currently in any fleet.
.s1
The specification of ships, <SHIP/FLEET>,
can have one of several syntaxes:
.NF
example     meaning
-------     -------
23          ship 23
2/14/23     ships 2, 14, and 23
c           all ships currently in fleet `c'
~           all ships currently in the \*Qnull\*U fleet
2,3         all ships in sector 2,3
-1:3,0:2    all ships in the square area bounded by -1,0 & 3,2
.FI
All fleets, (with the exception of the `~' fleet),
are limited to some maximum size
and you will be informed how many ships can be added
when this command is run.
.s1
Having ships organized into fleets can be very helpful in
loading, moving, etc.,
in that fewer commands are required to perform these commands
on groups of ships if they can be specified by fleet number.
.s1
Note that you can remove ships from a fleet by adding them to
the `~' fleet.  e.g.
.EX fleetadd ~ A
This command would purge all ships from fleet `A'.
.s1
.SA "load, lookout, navigate, radar, ship, unload, Ships"
