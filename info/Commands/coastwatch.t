.TH Command COASTWATCH
.NA coastwatch "Check from sectors for nearby ships"
.LV Basic
.SY "coastwatch [<SECTS>]"
The coastwatch command allows coastal sectors to report sightings of ships.
.s1
Visibility range is equal to
four times the sector efficiency
expressed as a decimal times the technology factor;
e.g. a 100% efficient harbor with a 50% technology factor can
see 2 sectors away.
An exception is radar installations, which have a maximum range
equal to fourteen times the sector efficiency, (i.e. 3.5 times as far).
.s1
A typical usage might be:
.EX coast #3 ?type=)
.NF
  3,-11 efficiency 100%, range 5.6
Pandora (#5) oil derrick #74 @3, -9
 -1,4   efficiency 100%, range 5.6
 -3,13  efficiency 100%, range 5.6
Galexis (#2) destroyer #35 @-3, 11
.FI
.s1
Note that subs will NOT show up on coastwatch, unless they are in a 
sector you own.
.s1
.SA "skywatch, lookout, Sector-types, radar, Ships, Detection"
