.TH Command BESTDISTPATH
.NA bestdistpath "Show the best distribution path from one sector to another"
.LV Expert
.SY "bestpath <SECTS> <SECTS>"
.SY "bestdistpath <SECTS> <SECTS>"
.s1
The bestpath & bestdistpath commands show you the best path from any sector 
to any sector. You must own both sectors.
.s1
The path shown by bestpath is the cheapest
owned existing path in terms of mobility.
.s1
Bestdistpath shows the best distribution path that can be found, which
should be the same as the best path..
During the update, sectors distribute to
and from their distribution point by means of the best distribution path
they can find at that time.
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
