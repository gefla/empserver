.TH Command EDIT
.NA edit "Change stats of country, ship, land unit, or plane"
.LV Expert
.SY "edit <country|land|unit|ship|plane|nuke> [<KEY> <VALUE>]..."
The \*Qedit\*U command allows deities to edit properties of a country,
sector (confusingly called <land> here), land unit, ship, plane or
nuke.
.s1
If you don't specify any <KEY> <VALUE> pair, \*Qedit\*U enters
interactive mode.  Editable properties are shown together with their
keys and values.  Enter a key character and a value to change a
property.  Hit return to finish the command.
.s1
You can also specify one or more <KEY> <VALUE> pairs as command
arguments.  In this case, \*Qedit\*U just sets the properties, and
does not enter interactive mode.
.SA "setsector, setresource, give, Deity"
