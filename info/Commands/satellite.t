.TH Command SATELLITE
.NA satellite "Download data from a reconnaissance satellite"
.LV Expert
.SY "satellite <PLANE> [<se|sh|l> [?cond&cond&...]]"
The satellite command is how you get information from your satellites.
A satellite must be in orbit (launched)
before it will begin transmitting.
The type of information you get from a satellite
depends on what type of satellite it is.
.s1
An ordinary satellite will return data that looks remarkably like
\*Qradar\*U output.
If the satellite has \*Qimaging\*U capabilities
you will see the sector designations.
If not then you will see a '?' for inhabited sectors.
.s1
A satellite with imaging capabilities
will also show ships and land units.
.s1
Spy satellites provide much more information.
Their output is a table that looks remarkably like \*Qspy\*U output
followed by another table listing all the ships in the area with
their owner, ship type, and efficiency.
.s1
Spy satellites with \*Qimaging\*U capabilities can also see subs!
.s1
After launching, a satellite will take a while to come \*Qon line\*U.
During this period,
(shown by mobility being < 127),
you cannot use the satellite.
.s1
If a satellite has been damaged, then it will only transmit partial
data.  For example, a 75% satellite will only display three out of
every four sectors.  Note that it will always be the same 75% of
sectors which are displayed (i.e. it is not random), so you will gain
nothing by repeatedly downloading data from the satellite.
.s1
You may also filter the information you receive from the satellite
using conditionals.  Here are some examples to show you how
conditionals work.  In all of the following examples, we will assume
that we are using satellite #28.
.s1
To list all airports:
.EX "sat 28 se ?des=*"
To list all airports owned by country #7:
.EX "sat 28 se ?des=*&own=7"
To list all ships in 12,14:
.EX "sat 28 sh ?xloc=12&yloc=14"
To list all land units:
.Ex "sat 28 l"
To list all cavalry units:
.EX "sat 28 l ?type=cavalry".
.s1
For a complete list of conditionals you can use, see info Syntax.
Note that the large radar map is only drawn when the output is not filtered:
.EX "sat 28"
.s1
.SA "launch, radar, spy, census, map, Syntax, Ship-types, Planes, Detection"
