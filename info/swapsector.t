.TH Command SWAPSECTOR
.NA swapsector "Swap two sectors"
.LV Expert
.SY "swapsector <SECT> <SECT>"

This command allows the deity to pick up two sectors and then put them
back down in each-other's location.  It is particularly handy for
changing the shape of start islands when setting up a game.  For
example, a piece of an island may be cut off by a mountain, and so to
fix it you could just swap the mountain over to some other part of the
island.  All aspects of the sector (e.g. resources, commodities) get
transferred to the new location.  The only change that is made is that
dist paths are wiped.

.SA "give, setsector, setresource, Deity"
