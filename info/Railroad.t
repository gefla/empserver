.TH Concept "Railroad"
.NA Railroad "How railroads work"
.LV Expert
Trains are land units with capability \*Qtrain\*U.  They can't be
loaded on land units, but they can be loaded on ships, subject to the
usual restrictions for non-light and heavy land units.
.s1
Unlike other units, trains can enter a sector only if it has
(operational) railroad track.
.s1
If option RAILWAYS is disabled, a sector has railroad track as long as
its rail infrastructure efficiency is non-zero.  Train mobility cost
depends on rail infrastructure efficiency (see \*Qinfo Mobility\*U).
Spy and satellite reports show approximate rail infrastructure
efficiency in column \*Qrl eff\*U.
.s1
If option RAILWAYS is enabled, all highway-like sectors are railways,
and the track is operational as long as the sector is at least 5%
efficient.  A sector is highway-like if its mobility cost at 100% is
zero (column mob cost in \*Qshow sect s\*U).  Operational railways
additionally extend track into adjacent sectors that are at least 60%
efficient and owned by the same nation.  Sector selector track gives
the number of operational railways within one sector range.  Spy and
satellite reports show the presence of track in column \*Qrl eff\*U.
To visualize your railway network, try \*Qsect # ?track#0\*U.
.s1
.SA "LandUnits"
