.TH Command STOP
.NA stop "Stop production and repairs"
.LV Expert
.SY "stop <TYPE> <SECTS|SHIPS|PLANES|UNITS|NUKES>"
The stop command stops production and repairs for one update.
.s1
Stopped sectors don't gain efficiency, don't produce anything, and
don't work on ships, planes or units there, but otherwise function
normally: they still pay tax, distribute goods and so forth.
.s1
Stopping ships, planes and land units works just the same: they don't
gain efficiency, they don't produce anything (if applicable), and
they don't work on embarked land units or planes.
.s1
Stopping nukes doesn't do anything in particular, right now.
.s1
Conquering a sector, ship, plane, land unit or nuke stops it
automatically.  Stop orders expire at the
update.  Use the start command to countermand them manually.
.s1
.EX stop se 2,0
.NF
Wed Jan 20 21:35:34 1993
PRODUCTION STOPPAGE
  sect        eff
   2,0    c  100% will not produce or gain efficiency.
    1 sector
.FI
.s1
.SA "start, census, Producing, Updates, Planes, Ships, LandUnits, Nukes"
