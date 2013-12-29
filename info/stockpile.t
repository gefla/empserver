.TH Command STOCKPILE
.NA stockpile "Designate members of a \*Qstockpile\*U"
.LV Expert
.SY "stockpile <STOCKPILE> <NUKE/STOCKPILE>"
The stockpile command is used to specify the stockpile groupings
of your nukes.
.s1
.EX stockpile <STOCKPILE> <NUKE/STOCKPILE>
In the syntax <STOCKPILE> is the alphabetic character to be used as the
stockpile designation.
This character can be chosen from the set of
upper or lower case a-z and tilde (~).
The pseudo-stockpile specification tilde
specifies all nukes not currently in any stockpile.
.s1
The specification of nukes, <NUKE/STOCKPILE>,
can have one of several syntaxes:
.NF
example     meaning
-------     -------
23          nuke 23
2/14/23     nukes 2, 14, and 23
c           all nukes currently in stockpile `c'
~           all nukes currently in the \*Qnull\*U stockpile
2,3         all nukes in sector 2,3
-1:3,0:2    all nukes in the square area bounded by -1,0 & 3,2
.FI
All stockpiles, (with the exception of the `~' stockpile),
are limited to some maximum size
and you will be informed how many nukes can be added
when this command is run.
.s1
Having nukes organized into stockpiles can be very helpful in
loading, moving, etc.,
in that fewer commands are required to perform these commands
on groups of nukes if they can be specified by stockpile number.
.s1
Note that you can remove nukes from a stockpile by adding them to
the `~' stockpile.  e.g.
.EX stockpile ~ A
This command would purge all nukes from stockpile `A'.
.s1
.SA "nuke, transport, Nukes"
