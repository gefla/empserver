.TH Command SHOW
.NA show "Characteristics of sectors, units, bridges, items and news"
.LV Basic
.SY "show <land|nuke|plane|sector|ship> [b|s|c] [tech]"
.SY "show <bridge|item|news|tower>"
The show command shows characteristics of sectors, units (ships,
planes, land units and nukes), bridges (spans and towers), items and
news.
.s1
The format of each output is more fully described in separate info
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
              blst dam lbs tech $
10kt fission     3  80   4  200 $7500
50kt fission     3 100   6  225 $9000
100kt fission    4 120   8  250 $12000
250kt fusion     4 150   4  280 $7500
500kt fusion     5 170   6  295 $9000
1mt fusion       6 190   8  310 $12000
3mt fusion       7 210  10  325 $19000
.FI
.s1
.SA "Ship-types, Nuke-types, Plane-types, Unit-types, Sector-types, Ships, Planes, LandUnits, Nukes, Bridges, Item-types, newspaper"
