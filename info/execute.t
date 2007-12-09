.TH Command EXECUTE
.NA execute "Execute a batch file"
.LV Expert
.SY "execute <INPUT FILE>"
This command reads Empire commands from <INPUT FILE> and executes them.
.s1
You can't execute a batch file from within a batch file at this time.
Redirections in batch files are not supported either.
.s1
Note: The execute protocol is broken by design.  Not all clients
support it.
Some clients get confused by funny file names, or redirections.
.SA "Playing"
