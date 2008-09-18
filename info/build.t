.TH Command BUILD
.NA build "Build ships, planes, land units, nuclear weapons or bridges"
.LV Basic
.SY "build [ship|plane|land|nuke] <SECTS> TYPE [NUMBER] [TECH] [SURE?]"
.SY "build [bridge|tower] <SECTS> DIRECTION"
.s1
The build command lets you build ships, planes, land units and nukes
in your harbors, airfields, headquarters and nuclear plants.  It is
also used to build bridges.
.s1
To build stuff, you generally need available work, raw materials
(e.g. light and heavy construction materials, oil, radioactive
materials), cash, and technology, depending on the thing being built.
For nukes, you may also need research, depending on game
configuration.
Try commands \*Qversion\*U and \*Qshow\*U to learn
how your game is configured.
.s1
A sector's available work is work not used up by the update (building
efficiency, working on things, etc).  It is shown in the output of the
\*Qcensus\*U
command in the 'avail' column.
.s1
If you give multiple sectors, the build command applies to each of
them.
.s1
Except for bridges, you use the TYPE argument to order what to build.
Choices depend on your tech level; check out the \*Qshow\*U command to
find out more.  You can order a number of things to be built by giving
the optional NUMBER argument.  If you ask to build more than 20, the
build command will ask for confirmation.  The optional argument SURE?
lets you suppress that.
.s1
Normally, build builds stuff at your current tech level.  The optional
argument TECH lets you build at a lower tech level.
.s1
HARBORS & BUILDING SHIPS
.s1
Ships are built in harbor sectors. 20% of the materials listed by
\*Qshow\*U must be available at the time of building.  Ships first
appear at 20% efficiency (less efficient ships sink instantly). Then,
each update, the ship will grow in efficiency, and use up more of the
required materials until it reaches 100%.  For example:
.s1
.EX build s 6,2 frg 2
.NF
frg  frigate (#13) built in sector 8,0
frg  frigate (#14) built in sector 8,0
That just cost you $240.00
.FI
.s1
In addition to the avail in the harbor sector, ships are also worked
on by their crews.  Only military crew can work on a military ship
(one that can fire guns).  Civilian ships can employ both civilian and
military crew.  So, for maximum efficiency growth,
put full crews on your newly built
ships and leave them in harbor until they reach 100%.
.s1
The work required to add a point of efficiency to a ship is
.s1
.NF
(20 + (lcm_to_build + 2 * hcm_to_build))/100
.FI
.s1
Ships at sea have only their crews to make repairs.
A small crew on a large ship
may not be able to make any repairs at all.
(Moral: keep your ships fully crewed in anticipation of future damage.)
Repairs at sea do not use any lcms or hcms, but may not push the
efficiency of a ship over 80%.
.s1
AIRFIELD SECTORS & BUILDING PLANES
.s1
Planes are built in airfields. 10% of the materials
listed by \*Qshow\*U must be available at the time of building.
Planes first appear at 10% efficiency.
Then, each update, the plane will gain efficiency and use up more of
the required materials until it reaches 100%.
A plane is not capable of leaving the ground until
it has reached 40% efficiency.  Planes below 10% efficiency are
destroyed.
.s1
The work required to add a point of efficiency to a plane is
.s1
.NF
(20 + (lcm_to_build + 2 * hcm_to_build))/100
.FI
.s1
Planes will also gain efficiency in non-airport sectors, but at only 1/3rd
the normal rate.
.s1
Planes on carriers will also gain
efficiency. The amount of work available is based on the carrier's crew,
and the rate is the same as a non-airport sector.
Repairs at sea do not use any materials, but may not push the
efficiency of a plane over 80%.
.s1
HEADQUARTERS SECTORS & BUILDING LAND UNITS
.s1
Land units are built in headquarters sectors.
In order to build a land unit, you must have 10% of the items
listed by \*Qshow\*U available at the time of building. The land unit will
appear at 10% efficiency (a unit of less than 10% efficiency is
dead). Then,
each update, the unit will grow in efficiency, and use up more
of the required goods until it reaches 100%.
.s1
The work required to add a point of efficiency to a land unit is
.s1
.NF
(20 + (lcm_to_build + 2 * hcm_to_build))/100
.FI
.s1
Land units can also gain efficiency in fortress sectors, but cannot
be built there.
Land units can gain efficiency in any other sector type (assuming
the correct goods are available), but at 1/3rd the normal rate.
.s1
NUCLEAR PLANT SECTORS & BUILDING NUKES
.s1
Nuclear devices are built in nuclear plants.  All of the materials
listed by \*Qshow\*U must be available at the time of building.  New nukes
appear fully operational.
.s1
BUILDING BRIDGES
.s1
A bridge is built in a sea sector by an adjacent sector.
.s1
If option EASY_BRIDGES is disabled, only bridge head sectors can build
bridge spans.  Otherwise, any (coastal) sector can build bridge spans,
but the new bridge span must be adjacent to a land sector or a bridge
tower.  If option BRIDGETOWERS is enabled, bridge spans can build
bridge towers.  Bridge towers are possible only in open water,
i.e. not adjacent to land.
.s1
In any case, the building sector must be at least 60% efficient.  To
find out the required amounts of materials, avail, capital and
technology, use commands \*Qshow bridge build\*U and \*Qshow tower
build\*U.
.s1
If you type
.EX build b 1,3
and 1,3 has enough hcm (heavy construction
materials), the workforce in the sector is large enough (as defined above),
you have enough money, and your technology level is high enough,
then the program will respond:
.NF
Bridge head at 1,3
 + k
o a .
 . .
build span in what direction? n
.FI
.s1
If everything is correct, you get:
.NF
Bridge span built over 2,4
.FI
Of course, you may also build bridges all in one line as in:
.EX build b 1,3 n
.s1
A sector that has a bridge built in it is automatically cleared
of mines at the time of building.
.s1
Bridges are built at 20%.  If the efficiency drops below 20% (due to
bombing, shelling, etc.) the bridge falls.  A bridge also falls if all
its supporting sectors are reduced below 20%.  A sector is a
supporting sector if it is of a type that can build bridges.  Bridge
heads and bridge towers are.  With option EASY_BRIDGES, all land
sectors are as well.
.s1
When a bridge falls, all mines in the sector are cleared.
.s1
Ships can navigate under bridge spans that are at least 60% efficient.
.s1
.SA "Unit-types, Ship-types, Plane-types, Nuke-types, Bridges, show, upgrade, Maintenance, Ships, Planes, LandUnits, Nukes, Sectors"
