.TH Command WINGADD
.NA wingadd "Designate members of a \*Qwing\*U"
.LV Expert
.SY "wingadd <WING> <PLANE/WING>"
The wingadd command is used to specify the wing groupings
of your planes.
.s1
.EX wingadd <WING> <PLANE/WING>
In the syntax <WING> is the alphabetic character to be used as the
wing designation.
This character can be chosen from the set of
upper or lower case a-z and tilde (~).
The pseudo-wing specification tilde
specifies all planes not currently in any wing.
.s1
The specification of planes, <PLANE/WING>,
can have one of several syntaxes:
.NF
example     meaning
-------     -------
23          plane 23
2/14/23     planes 2, 14, and 23
c           all planes currently in wing `c'
~           all planes currently in the \*Qnull\*U wing
2,3         all planes in sector 2,3
-1:3,0:2    all planes in the square area bounded by -1,0 & 3,2
.FI
All wings, (with the exception of the `~' wing),
are limited to some maximum size
and you will be informed how many planes can be added
when this command is run.
.s1
Having planes organized into wings can be very helpful in
loading, moving, etc.,
in that fewer commands are required to perform these commands
on groups of planes if they can be specified by wing number.
.s1
Note that you can remove planes from a wing by adding them to
the `~' wing.  e.g.
.EX wingadd ~ A
This command would purge all planes from wing `A'.
.s1
.SA "plane, fly, Planes"
