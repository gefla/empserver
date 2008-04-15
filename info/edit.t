.TH Command EDIT
.NA edit "Edit country, sector, ship, plane or land unit"
.LV Expert
.SY "edit <country|land|unit|ship|plane> <NAT|SECT|SHIP|PLANE|LAND> [<KEY> <VALUE>]..."
The \*Qedit\*U command allows deities to edit properties of a country,
sector (confusingly called <land> here), ship, plane or land unit.
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
.SA "setsector, setresource, give, Deity"
