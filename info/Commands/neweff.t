.TH Command NEWEFF
.NA neweff "Find out what the efficiency of a sector will be after the next update"
.LV Expert
.SY "neweff <SECTS>"
Neweff predicts how efficient a sector or sectors will be after the
next update.
.s1
In the syntax of the neweff command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
.s1
A typical usage might be:
.EX neweff 2,0
.NF
EFFICIENCY SIMULATION
   sect  des    projected eff
   2,0    +     18%
    1 sector
.FI
.s1
.SA "Innards, Sectors, Update"
