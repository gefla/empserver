.TH Command DISABLE
.NA disable "Disable Updates"
.LV Expert
.SY "disable"
No updates will fire if you disable them.  "UPDATES ARE DISABLED" will
show up in the output of the "update" command.
.s1
Disabling updates this way does not stop the Empire clock.  Stuff
using the clock, like BTU growth, continues normally.  Should a
scheduled update be actually missed, however, the Empire clock
automatically recalibrates, to maintain the fixed number of ETUs
between updates.  This effectively stops the clock.
.s1
You should not use this command as a tool for implementing your update
schedule!  Enabling and disabling updates by hand is far too error
prone, and your players can't trust the update command to find out
when the next update really is.  Use the update schedule file instead.
.SA "update, enable, Time, Deity"
