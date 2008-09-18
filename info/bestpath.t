.TH Command BESTPATH
.NA bestpath "Show the best path from one sector to another"
.LV Expert
.SY "bestpath <SECTS> <SECTS>"
.s1
The bestpath command shows you the best path from any sector
to any sector. You must own both sectors.
.s1
The path shown by bestpath is the cheapest
owned existing path in terms of mobility.
.s1
Note that if you give these commands areas, they will show you more paths.
For example,
.s1
.EX bestpath #1 #2
.s1
will show the best paths from all sectors in realm 1 to all the sectors in
realm 2.
.s1
.SA "distribute, Maps"
