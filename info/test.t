.TH Command TEST
.NA test "Check the effect of a proposed move"
.LV Expert
.SY "test <ITEM> <SECT> <NUMBER> <ROUTE|DESTINATION>"
The test command lets you check the effect of a proposed move.
This can be helpful when you're not sure whether you have enough
mobility to reach a certain location or you wish to compare two
possible routes for a move.
.s1
The test command has exactly the same arguments as the move command
and acts almost the same.
There are two important differences:
.in +0.3i
.s1
Nothing is actually moved and no mobility is used.
.s1
The move is tested for <NUMBER> <ITEM> irrespective of how much
<ITEM> is actually in the sector.  This is useful for testing a sector
which will be used as a temporary stopover in a multi-step move to
make sure the stuff won't get stranded there.
.in -0.3i
.s1
.SA "move, Commodities, Transportation"
