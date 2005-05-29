.TH Concept "Moving Things About"
.NA Moving "A quick reference for moving things around"
.LV Basic
.s1
Most of Empire consists of you moving something from one place to
another.  The following table tells you which commands you use
to move the kind of thing that you want to move.
.s1
.NF
What            From        To       Command
----------------------------------------------------
comm            sector      sector      move <WHAT> <FROM> <NUM> <TO>
comm/land/plane sector      ship        load <WHAT> <SHIP> <NUM>
comm/plane      sector      unit       lload <WHAT> <UNIT> <NUM>
comm/land/plane ship        sector    unload <WHAT> <FROM> <NUM>
comm/land       ship        ship        tend <WHAT> <FROM> <NUM> <TO>
comm            ship        unit       ltend <WHAT> <FROM> <NUM> <TO>
comm/plane      unit        sector   lunload <WHAT> <FROM> <NUM>
unit            sector      sector     march <UNITS>
ship            sector      sector      navi <UNITS>
plane           sector      sector     trans <PLANE> <TO>
nuke            sector      sector     trans <FROM> <TYPE> <NUM> <TO>
nuke            sector      plane        arm <PLANE> <TYPE>
nuke            plane       sector    disarm <PLANE>
.FI
.s1
Note that in the case of load, lload, unload, tend, and lunload, the
way you specify whether you are moving a commodity, land unit or plane
is by using the word "land" or "plane" instead of the commodity name
in the <WHAT> field.  If <WHAT> is "land" or "plane", then <NUM> is
the unit or plane id.  So for example, to load ship #1 with land unit
#2, you would type:
.EX "load land 1 2"
.s1
.SA "Mobility, Transportation"

