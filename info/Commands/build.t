.TH Command BUILD
.NA build "Build ships, planes, bridges, land units or nuclear weapons"
.LV Basic
.SY "build [land|ship|plane|nuke|bridge] <SECTS> [unit-type|ship-type|plane-type|nuke-type|direction] [Number to build] [tech] [sure?]"
The build command is used to specify the types of units to be built
at your headquarters, ships
to be built in your harbors, the planes and missiles at your airfields,
the nuclear devices at your nuclear production facilities,
and the directions in which bridge spans are to be built.
.s1
The ability to produce units, ships, planes, nukes, or bridges, is based on
several factors:
labor force, light production units, heavy production units,
oil units, radioactive material units, available cash,
and technology level.
The quantities or levels required vary with the object being built.
.s1
In order to build anything, the sector involved needs to have
avail. Avail is work available in the sector after the demands
of the update (building efficiency, working on things, etc)
Avail in the sector is show in the output of the \*Qcensus\*U
command in the 'avail' column.
.s1
The "Number to build" option may only be given if the command is being
specified totally on the command line. If you give multiple sectors and
a number to build, it will attempt to build that number at EACH sector
specified.  If you ask to build more than 20 units, it will ask you
"are you sure you want to build so many?".
.s1
If you don't specify TECH then it will build the unit at your current
tech level.
.s1
BUILDING BRIDGES
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
With the EASY_BRIDGES option enabled, bridges may be built from any sector
that is at least 60% efficient.  Otherwise, bridges may only be built
from bridge-head sectors.  The
sector the bridge span is to be built in must be adjacent to
at least 1 non-sea, non-bridge span sector.
.s1
If everything is correct, you get:
.NF

Bridge span built over 2,4
.FI
To find out the required amounts of materials, workforce, and money
use the \*Qshow bridge build\*U command.
.s1
You may now also build bridges all in one line as in:
.EX build b 1,3 n
.s1
A sector that has a bridge built in it is automatically cleared
of mines at the time of building.
.s1
Bridge spans are built at 20%.  If the efficiency drops below 20% (due
to bombing, shelling, etc.) the bridge span falls.
.s1
If a bridge falls, all mines in the sector are likewise cleared.
.s1
BUILDING BRIDGE TOWERS
.s1
If you type
.EX build t 1,3
and 1,3 has enough hcm (heavy construction
materials), the workforce in the sector is large enough (as defined above),
you have enough money, and your technology level is high enough, and the
sector is a bridge span,
then the program will respond:
.NF
Building from 1,3
 + a
o = .
 . .
build tower in what direction? n
.FI
.s1
Bridge towers can only be built if the BRIDGETOWERS option is enabled. 
.s1
In addition, bridge towers can only be built in open water, i.e. not 
adjacent to any land or other bridge towers.  Bridge towers must be
built from bridge spans that are at least 60% efficient.
.s1
If everything is correct, you get:
.NF

Bridge tower built in 2,4
.FI
To find out the required amounts of materials, workforce, and money
use the \*Qshow tower build\*U command.
.s1
You may now also build bridge towers all in one line as in:
.EX build t 1,3 n
.s1
A sector that has a bridge tower built in it is automatically cleared
of mines at the time of building.
.s1
Once a bridge tower is built, you may build other bridge spans from it.
.s1
Note that you cannot navigate through a bridge tower like you can through a
bridge span.
.s1
Bridge towers are built at 20%.  If the efficiency drops below 20% (due
to bombing, shelling, etc.) the bridge tower falls.
.s1
If a bridge tower falls, all mines in the sector are likewise cleared.
In addition, if a bridge tower falls, all bridges around it are
knocked down unless they have another supporting bridgehead (such as
another tower, bridge head or next to land if EASY_BRIDGES is on.)
.s1
NUCLEAR PLANT SECTORS & BUILDING NUKES
.s1
If you type
.EX build 6,2
and 6,2 is a nuclear plant that has enough hcm
(heavy construction materials),
lcm (light construction materials), oil, and radioactive material,
the workforce in the sector is large enough (as defined above),
you have enough money, and your technology level is high enough,
then the program will construct a new nuclear device of the type
requested, and add it to the current stockpile in that sector.
.s1
Note: If the DRNUKE option is enabled, you will need a certain amount
of research to make nukes. At the present time, it is 1/3 the amount of 
tech needed to make the nuke. See the \*Qshow\*U command, which lists 
this if applicable.
.s1
TYPES OF THINGS YOU CAN BUILD (besides bridges)
.s1
Many different types of units/ships/planes/nukes are available.
For a brief table indicating the production costs of the
different things, respond with \*Q?\*U when asked for the type,
or use the \*Qshow\*U command.
Note that if there are two things in the list of items that are
similar, such as "destroyer 1" and "destroyer 2", you will need
to give the full name, or the program will build the first one
that it encounters.
.s1
\*QNote:\*U if you wish to give the ship-type argument on the
command line, it may be necessary to quote the ship-type. For
example: build s 0,0 "lander 2" 80 would build a lander 2. Without
the quotes, it might build a lander 1 instead, since the ship-type
is two words.
.s1
Once built, units/ships/planes grow in efficiency until they reach
100%. This growth is accomplished using the avail left in the sector
at the time of the update, so if you use all the avail building things,
there won't be any left to make them more efficient. The avail there
is divided amongst all things in the sector needing work, so the more
units/ships/planes there, the less each will be worked upon.
Ships/planes/units also require materials be present in the sector to
gain efficiency. In general, for each 1% the ship/plane/unit gains in
efficiency, it will use 1% of the materials listed by \*Qshow\*U.
.s1
HEADQUARTERS SECTORS & BUILDING LAND UNITS
.s1
Land units are built in headquarters sectors. (designation !).
In order to build a land unit, you must have 10% of the items
listed by \*Qshow\*U (typically military, lcms, hcms, guns, shells,
petrol, etc) available at the time of building. The land unit will
appear at 10% efficiency (a unit of less than 10% efficiency is
dead). Then, 
each update, the unit will grow in efficiency, and use up more
of the required goods until it reaches 100%.
Land units can also gain efficiency in fortress sectors, but cannot
be built there.
.s1
The work required to add a point of efficiency to a land unit is 
.s1
.NF
(20 + (lcm_to_build + 2 * hcm_to_build))/100
.FI
.s1
A land unit will can gain efficiency in any other sector type (assuming
the correct goods are available), but at 1/3rd the normal rate.
.s1
HARBORS & BUILDING SHIPS
.s1
Ships are built in harbor sectors. 20% of the materials
listed by \*Qshow\*U must be available at the time of building.
(typically hcms & lcms)
Ships first appear at 20% efficiency and a 19% efficient ship
will not float (i.e., it sinks). Then, each update, the ship will
grow in efficiency, and use up more of the required materials until
it reaches 100%.
.s1
In addition to the avail in the harbor sector, ships are also
worked on by their crews.  So, for fastest efficiency growth,
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
Crew members each supply the same amount of work,
whether civilian, military, or uncompensated worker.
A small crew on a large ship
may not be able to make any repairs at all.
(Moral: keep your ships fully crewed in anticipation of future damage.)
Repairs at sea do not use any lcms or hcms, but may not push the 
efficiency of a ship over 80%.
.s1
Entropy acts on ships without sufficient crews. In general, if a ship
is at sea, and has less than about 40% of its maximum crew, it
will gradually rust, take on water, and sink.
.s1
.NF
	The amount of efficiency lost depends on the ship size and crew level.

	Figure the following quantities:

	Avail is the work done by the crew: 

		On a military ship: work = etus * mil/2
		On a civilian ship: work = etus * (civ/2 + mil/5)

	Crew is the current # of mil (for a military ship), or civs for a
civilian ship.

	Max crew is the max # of mil/civs (depending on ship type)

	W_p_eff is the amount of work per point of efficiency of the ship:

		w_p_eff = 20 + (lcm + 2 * hcm);

	Lcms & hcms are the cms required to build the ship.

	Then,  subtract (etus * (100-((crew*100)/max_crew)))/7 from the avail.

	If the result is less than 0, divide it by the w_p_eff, and subtract
	that amount from the ship's efficiency.

	avail -= (etus * (100-((amt*100)/abs_max)))/7;
	ship->shp_effic += avail/w_p_eff;
	Example: A frigate (a military ship) can hold 60 mil. It has only 10.
	
	w_p_eff = 20 + (lcm + 2*hcm) = 20 + (30 + 2*30) = 110
	Avail = etus * mil/2 = 60 * (10/2) = 300

	300 - (60 * (100-((10*100)/(60)))/7) ~= -420

	Since the result is negative, divide it by w_p_eff:

	420/110 = 3.8 = 4% loss of efficiency

	Note from this that the frigate would need 24 crew to break even.
.FI
.s1
AIRPORT SECTORS & BUILDING PLANES
.s1
The choices range from low tech \*Qfighter 1\*U planes
up to and including super-long range \*Qjet hvy bomber\*Us
and \*Qicbm\*Us.
For a brief table indicating the production costs of the
different plane types respond with \*Q?\*U, or check out
the \*Qshow\*U command.
.s1
Note that planes first appear at 10% efficiency.
10% of the listed hcms/lcms/crew must be in the airport in order to
build the plane. (typically hcms, lcms, and mil (crew))
In order for the plane to gain efficiency, it needs
hcms/lcms/mil, as ships and land units do.
As time passes, and as long as there are \*Qavailable work units\*U in the
airport at which the planes reside (to work on the planes), they grow to
100% efficiency.  A plane is not capable of leaving the ground until
it has reached 40% efficiency.
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
If the CARRIER_WORK option is enabled, planes on carriers will also gain 
efficiency. The amount of work available is based on the carrier's crew,
and the rate is the same as a non-airport sector.
Repairs at sea do not use any lcms/hcms/mil, but may not push the 
efficiency of a plane over 80%.
.s1
Once you have reached tech level 290, you are no longer allowed to
build planes that have tech level below 151.
.s1
.SA "Unit-types, Ship-types, Plane-types, Nuke-types, Bridges, show, upgrade, Maintenance, Ships, Planes, LandUnits, Sectors"
