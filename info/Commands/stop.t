.TH Command STOP
.NA stop "Stop sector production"
.LV Expert
.SY "stop <SECTS>"
The stop command orders a sector to stop producing for an update.
After the update, the sector will return to normal.
.s1
The sector will still distribute, etc, but will not gain in efficiency or
produce anything. Ships, planes, and units in the sector will
not gain efficiency.
.s1
.EX stop 2,0
.NF
Wed Jan 20 21:35:34 1993
PRODUCTION STOPPAGE
  sect        eff
   2,0    c  100% will not produce or gain efficiency.
    1 sector
.FI
.s1
.SA "start, census, Production, Update"
