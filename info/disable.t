.TH Command DISABLE
.NA disable "Disable Updates"
.LV Expert
.SY "disable"
No updates will fire if you disable them.  The \*Qshow updates\*U
command will report the fact.
.s1
Disabling updates this way does not stop the Empire clock.  Stuff
using the clock, like BTU growth, continues normally.  Should a
scheduled update be actually missed, however, the Empire clock
automatically recalibrates, to maintain the fixed number of ETUs
between updates.  This effectively stops the clock.
.s1
You should not use this command as a tool for implementing your update
schedule!  Enabling and disabling updates by hand is far too error
prone, and your players can't trust the show command to find out
when the next update really is.  Use the update schedule file instead.
.SA "disable, show, Time, Deity"
