.TH Command THRESHOLD
.NA threshold "Set commodity distribution thresholds"
.LV Basic
.SY "threshold <ITEM> <SECTS> <THRESH>"
The threshold command is used to specify the level
of a given commodity which is to be maintained
by the sector's distribution sector.
These levels are maintained by either importing or exporting
commodities.
This importation and exportation is done automatically during
an update.
It uses much less mobility than manual moves.  See "info Mobility" for
details.
.s1
In the syntax,
<SECTS> is the sector or sectors from which the deliveries are made,
<ITEM> is one of the following:
.NF
c	civilians
m	military
u	uncompensated workers
f       food
s       shells
g       guns
p       planes
i       iron ore
d       gold dust
b       bars of gold
o       oil
l       light construction materials
h       heavy construction materials
r	radioactive materials
.FI
and <THRESH> is a number specifying the level of commodity which
is desired.
.s1
The census and commodity reports will indicate distribution thresholds
by the \*Qhundreds digit\*U of the threshold level.
In order to have a threshold take effect, it must be at least one.
.s1
Sectors importing or exporting items can be seen by the \*Qroute\*U
command (marked with a \*Q$\*U) or by the sect command.
.s1
Also, note that no distribution center will ship out its last civilian.
.s1
.SA "distribute, census, commodity, route, level, Distribution"
