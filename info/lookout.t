.TH Command LOOKOUT
.NA lookout "Check from ships for other ships and/or sector types"
.LV Basic
.SY "lookout <SHIP/FLEET>"
The lookout command allows ships to report sightings
of other ships and land sectors.
.s1
The various ranges over which lookout is effective
break down into two groups:
.s1
SHIP to SHIP
.s1
.in +0.2i
This range is dependent on both the visibility of the ship
being observed and the visual range of the observing ship.
(see \*Qinfo Ship-types\*U)
Submarines at sea are a special case - they cannot be seen by simply looking
out from a ship!  In order to detect subs at sea, you need to use sonar.
Submarines have a very limited range for what ships they
can see using their periscope--they can usually only see ships in the
same sector.  However, all ships (including subs) which are docked in
a harbour are visible to all ships (including subs) in adjacent sectors.
(see also \*Qinfo sonar\*U)
.in
.s1
SHIP to SHIP
.s1
.in +0.2i
This range is effectively 1.0.
.in
.s1
For example:
.EX lookout 24
.NF
Groonland (#1) patrol boat #10 @-13,7
Curmudgeon (#4) heavy cruiser #36 @-14,6
Groonland (#1) oil derrick #41 @-14,6
Groonland (#1) tender #53 @-14,6
Groonland (#1) mine 100% eff with approx. 300 civilians @-12,6
Groonland (#1) heavy manufacturing 23% eff with approx. 10 civilians @-11,7
.FI
.s1
.SA "coastwatch, Ship-types, Sector-types, radar, sonar, Ships, Detection"
