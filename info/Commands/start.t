.TH Command START
.NA start "Turn sector production on"
.LV Basic
.SY "start <SECTS>"
The start command orders a sector to start producing for an update.
This counteracts the stop command.
.s1
.EX start 2,0
.NF
Wed Jan 20 21:37:22 1993
PRODUCTION STARTING
  sect        eff
   2,0    c  100% will be updated normally.
    1 sector
.FI
.s1
.SA "stop, census, Production, Update"
