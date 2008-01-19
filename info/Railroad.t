.TH Concept "Railroad"
.NA Railroad "How railroads work"
.LV Expert
Trains are land units with capability \*Qtrain\*U.
.s1
Unlike other units, trains can enter a sector only if it has
(operational) railroad track.
.s1
If option RAILWAYS is disabled, a sector has railroad track as long as
its rail infrastructure efficiency is non-zero.  Train mobility cost
depends on rail infrastructure efficiency (see \*Qinfo Mobility\*U).
Spy reports show the rail infrastructure efficiency in colum \*Qrl
eff\*U.
.s1
If option RAILWAYS is enabled, all highway-like sectors are railways,
and the track is operational as long as the sector is at least 5%
efficient.  A sector is highway-like if its mobility cost at 100% is
zero (column mob cost in \*Qshow sect s\*u).  Operational railways
additionally extend track into adjacent sectors that are at least 60%
efficient.  Sector selector track gives the number of operational
railways within one sector range.  This number is also shown by spy
report column \*Qrl eff\*U.  To visualize your railway network, try
\*Qsect # ?track#0\*U.
.s1
.SA "LandUnits"
