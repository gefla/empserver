.TH Command MOBUPDATE
.NA mobupdate "turn on/off mobility updating"
.LV Expert
.SY "mobupdate <MINUTES|check>"
The mobupdate command will disable mobility updating for 'X' minutes.
Note that this only works if the MOB_ACCESS option has been enabled.
.s1
Using this command with 0 minutes will turn mobility updating back on.
.s1
Using this command with 'check' or a negative amount of minutes will
tell you when the mobility updating will be turned back on if it is
off, or that it is on if it is on.
.s1
.SA "update, Deity"
