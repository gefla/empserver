.TH Command SHOW
.NA show "Stats for building ships, planes, units, or bridges"
.LV Basic
.SY "show <sector|bridge|land|plane|ship|nuke> [build|stats|capabilities] [tech]"
The show command either gives information on the indicated unit types.
This information is gleaned directly from the current Empire
data structures so is always up to date.
The format of each output is more fully described
in the relevant info pages
(Ship-types, Nuke-types etc.).
.s1
Describe currently knows about bridges, planes, ships, land units, nukes,
and sectors at a given tech (up to 1.25 times your current tech.)
.s1
To get information on a particular topic type:
.EX show unit-type
Unless the unit is a bridge, a further parameter is needed to indicate
whether you wish information about construction of units, the statistics
of units or the capabilities & abilities of units.
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
.s1
Note that you can only get information on items that you are actually
technologically capable of building (within 1.25 times of your current
tech.)
.FI
.s1
To learn about the various build costs, stats, capabilities, and their 
meanings, see the info files below.
.s1
.SA "Ship-types, Nuke-types, Plane-types, Unit-types, Sector-types, Ships, Planes, LandUnits"
