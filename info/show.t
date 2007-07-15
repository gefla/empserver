.TH Command SHOW
.NA show "Characteristics of sectors, units, bridges, items and news"
.LV Basic
.SY "show <land|nuke|plane|sector|ship> [b|s|c] [tech]"
.SY "show <bridge|item|news|tower>"
.SY "show <updates> [number]"
The show command shows characteristics of sectors, units (ships,
planes, land units and nukes), bridges (spans and towers), items and
news, as well as information on updates.
.s1
Characteristics are more fully described in separate info
pages (Ship-types, Nuke-types, etc.).
.s1
For sectors and units, a
further parameter is required to indicate whether you wish information
about building (b), statistics (s) or capabilities (c) of the selected
type.
.s1
By default, unit characteristics are shown at 1.25 times your current
technology level.  You can ask for a lower level with the optional
tech parameter.  Units beyond that level are not shown; you can only
get information on units that you are already or almost capable of
building.
.s1
So, for example
.EX show nuke capabilities
will give something like
.NF
Printing for tech level '325'
              blst dam lbs tech res $        abilities
10kt  fission    3  70   4  280  93 $  10000 
15kt  fission    3  90   5  290  96 $  15000 
50kt  fission    3 100   6  300 100 $  25000 
100kt fission    4 125   8  310 103 $  30000 
5kt   fusion     2  80   1  315 104 $  12500 
75kt  fusion     3  90   3  320 106 $  20000 
.FI
.s1
.SA "Ship-types, Nuke-types, Plane-types, Unit-types, Sector-types, Ships, Planes, LandUnits, Nukes, Bridges, Item-types, newspaper, Updates"
