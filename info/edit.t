.TH Command EDIT
.NA edit "Edit sectors, ships, planes, land units or countries"
.LV Expert
.SY "edit land <SECTS> [<KEY> <VALUE>]..."
.SY "edit ship <SHIPS> [<KEY> <VALUE>]..."
.SY "edit plane <PLANES> [<KEY> <VALUE>]..."
.SY "edit unit <LANDS> [<KEY> <VALUE>]..."
.SY "edit country <NATS> [<KEY> <VALUE>]..."
The \*Qedit\*U command allows deities to edit properties of a sector
(confusingly called <land> here), ship, plane, land unit, or country.
.s1
If you don't specify any <KEY> <VALUE> pair, \*Qedit\*U enters
interactive mode.  Editable properties are shown together with their
keys and values.  Enter a key character and a value to change a
property.  Hit return to finish the command.
.s1
You can also specify one or more <KEY> <VALUE> pairs as command
arguments.  In this case, \*Qedit\*U just sets the properties, and
does not enter interactive mode.
.s1
.SA "give, swapsector, Deity"
