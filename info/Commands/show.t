.TH Command SHOW
.NA show "Characteristics of bridges, ships, planes, land units or sectors"
.LV Basic
.SY "show <bridge|land|nuke|plane|sector|ship|tower> [b|s|c] [tech]"
The show command describes the characteristics of bridge spans, bridge
towers, planes, ships, land units, nukes, and sectors for a
given technology level.
.s1
The format of each output is more fully described in separate info
pages (Ship-types, Nuke-types, etc.).
.s1
Except for bridge (bridge span) and tower (bridge tower), a
further parameter is required to indicate whether you wish information
about building (b), statistics (s) or capabilities (c) of the selected
type.
.s1
The tech parameter defaults to 1.25 times your current technology
level.
.s1
You can only get information on ships, planes, land units and
nukes that you are `almost' capable of building (within 1.25 times of
your current technology level).
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
.SA "Ship-types, Nuke-types, Plane-types, Unit-types, Sector-types, Ships, Planes, LandUnits, Products"
