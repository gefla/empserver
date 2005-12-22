.TH Command INFO
.NA info "Provide information on various topics"
.LV Basic
.SY "info [<SUBJECT|COMMAND|TOPIC|all>]"
.SY "help [<SUBJECT|COMMAND|TOPIC|all>]"
The Empire info pages are organized by subject.  To get a list of
subjects, type:
.EX info
or
.EX help
You can then type:
.EX info Subject
to get a list of Empire commands and info topics which are related
to the Subject that you chose.  Note that Subjects always start with a
capital letter (this is to distinguish them from commands which are
always lowercase).
.s1
To get info on the syntax of an Empire command, type:
.EX info command
where "command" is the command you're interested in.  Note that info
about Empire command syntax is always specified lower-case.
.s1
To get info about some Empire topic, type:
.EX info Topic
Where Topic is the name of some Empire topic that you're interested in.
.s1
Every Empire command and topic belongs to at least one subject, and in
some cases more than one.
.s1
.EX info all
This will list all info pages.  Warning: this list is long!
.s1
As of this writing, some clients shanghai the help command to provide
help on their own command line extensions, but fail to integrate
server help.  If the help command doesn't work for you, try the info
command instead.
.s1
.SA "Bugs, Playing"
