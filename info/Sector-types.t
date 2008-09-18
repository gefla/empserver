.TH Concept "Sector Types"
.NA Sector-types "Description of different sector types"
.LV Basic
.NF
 BASICS                   INDUSTRIES                MILITARY / SCIENTIFIC
 .  sea                   d  defense plant          t  technical center
 ^  mountain              i  shell industry         f  fortress
 s  sanctuary             m  mine                   r  research lab
 \e  wasteland             g  gold mine              n  nuclear plant
 -  wilderness            h  harbor                 l  library/school
 ~  plains                w  warehouse              e  enlistment center
 c  capital/city          u  uranium mine           !  headquarters
 p  park                  *  airfield                        
                          a  agribusiness           FINANCIAL
 COMMUNICATIONS           o  oil field              b  bank
 +  highway               j  light manufacturing    
 )  radar installation    k  heavy manufacturing
 #  bridge head           %  refinery            
 =  bridge span         
 @  bridge tower
.FI
.s1
.ce
BASICS
.s1
SEA - Sea sectors form natural barriers that can
only be crossed by ships or spanned by bridges.
You can't designate anything else to be sea,
nor can sea be designated to be anything else.
.s1
MOUNTAIN - Mountain sectors form another natural
barrier that cannot be redesignated; however, they can be
moved through, at great expense in terms of mobility.
In a pinch, mountains can be used as capital.  They generate
very few BTUs, and represent being 'holed up in the mountains',
perhaps like Pancho Villa.
Mountains typically hold only 1/10th of the civilians and
uncompensated workers that other sectors do.
.s1
SANCTUARY - Sanctuary sectors are created when
a new nation is created.
They are inviolate in that no one can attack or harm them.
This protection ends
when the new country first moves out of the sanctuary.
This is called \*Qbreaking sanctuary\*U.  See \*Qinfo break\*U for
details.
.s1
WASTELAND - This is the result of the explosion of a nuclear device.
Wastelands are uninhabitable forever.
.s1
WILDERNESS - Most of the world is wilderness
at the beginning of the game.
Wilderness has no particular attributes;
you will probably want to
designate it as something else once you own it.
.s1
PLAINS - Plains sectors form another natural barrier that cannot
be redesignated.  They can be occupied and moved through however.
Plains typically hold only 1/10th of the civilians and
uncompensated workers that other sectors do.
.s1
CAPITAL/CITY - These sectors can serve as your capital.  You'll be
quite helpless without a capital, as many commands require you to have
one.  Your capital is also your source of bureaucratic time units
(BTUs), see \*Qinfo BTU\*U.
A nation may only have one active capital at a time, although many
sectors may be designated as capitals.  Use the capital command to
make a capital the active one.
The capture of a capital has disastrous consequences, which are
described in \*Qinfo capital \*U.
Capitals are typically better at defending against attack than other
sectors, except fortresses.
.s1
PARK - Parks are provided solely for the convenience of the
people.
Parks require construction materials to provide maintenance
services and
add to a nation's \*Qhappiness\*U level.
.s1
.ce
COMMUNICATIONS
.s1
HIGHWAY - Moving stuff through sectors costs mobility.  Efficienct
highways cost much less than other sectors.  To find out more about
moving stuff, start with \*info Transportation\*.
.s1
RADAR - Radar stations can scan the surrounding
area, and generate
a radar plot identifying sector types at distances up to 1/3
their range and ships up to a distance depending on ship visibility.
.s1
BRIDGE HEAD - Bridge heads are the land based ends of bridges.
They let you build bridge spans; see \*Qinfo build\*U for how.
If option EASY_BRIDGES is enabled, any sector can build bridges, not
just bridge heads.
.s1
BRIDGE SPAN - A bridge span is the suspended part of a bridge that
crosses water sectors.  Bridge spans are built and supported by bridge
heads and are much like highways except for two things:
.br
(1) Bridge spans must maintain at least 20% efficiency or else they
collapse.
.br
(2) If the only bridge head supporting a particular bridge span is
redesignated as something else, the bridge span will collapse, unless
option EASY_BRIDGES is enabled.
.s1
BRIDGE TOWER - Bridge towers are like bridge heads you can build
out in the ocean.  They are only available if option BRIDGETOWERS is
enabled.  You build bridge towers from a bridge span into open water,
not adjacent to any land or other bridge towers.  From a bridge tower,
you can build other bridge spans.
Bridge towers must maintain at least 20% efficiency or else they
collapse.
.s1
.ce
FINANCIAL
.s1
BANK - Banks are used for smelting and storing gold bars.
They include a smelter to refine gold dust into gold bars.
While the bars are stored in the bank the busy little bankers
invest them and return a profit on their use.
Banks are of the Fort Knox variety; they are more impervious to shelling
than most other sectors, and military in them fight harder
against attack as those in industries.
Banks are also particularly adept at moving gold bars around;
fewer mobility units
are required to move a gold bar from a bank than from an ordinary sector.
.s1
.ce
INDUSTRIES
.s1
DEFENSE - Defense plants turn construction materials into guns.
Guns are required to equip fortresses, warships, some land units and
for sector flak.
.s1
SHELL INDUSTRY - These sectors are similar to defense
plant sectors except they turn construction materials into
shells.
Most of anything that goes boom expends shells in the process.
.s1
MINE - Mines produce iron ore by digging it out of the ground.
Iron is required to make construction materials.
.s1
GOLD MINE - Gold mines are similar to ordinary
mines, except that they produce gold dust.
Gold dust is required to make gold bars, which are an important source
of income.
.s1
URANIUM MINE - Uranium mines are very similar to gold
mines, except that they produce radioactive materials.
.s1
HARBOR - Harbors combine shipyard facilities and docks.
This is where you build ships; see \*Qinfo build\*U for how.  Harbors
also repair incomplete or damaged ships, provided there's enough work
and materials.
Harbors are good at moving commodities, and are often used as
distribution centers.
.s1
WAREHOUSE - Warehouses are used to store commodities.  They are good
at moving commodities, and are often used as distribution centers.
.s1
AIRFIELD - This is where you build planes; see \*Qinfo build\*U for
how.  Airfield also repair incomplete or damaged planes, provided
there's enough work and materials.  Planes can take off and land at
airfields.  Some can take off and land in other sectors as well, and
some can also use carriers; see \*Qinfo Plane-types\*U for details.
.s1
AGRIBUSINESS - These sectors are large farms and provide food.
Agribusiness sectors produce much more food than other sectors.
.s1
OIL - Oil fields produce oil by pumping it out of the ground.
.s1
REFINERY - Refineries produce petroleum by processing oil.
.s1
LIGHT - Light manufacturing plants produce light construction materials
from iron.
.s1
HEAVY - Heavy manufacturing plants produce heavy construction materials
from iron.
.s1
.ce
MILITARY / SCIENTIFIC
.s1
FORTRESS - Fortresses are military strongpoints, resistant to attack,
adept at counter-attack.  They can fire guns (unless option
NO_FORT_FIRE is enabled), and they repair damaged land units, provided
there's enough work and materials.
.s1
TECHNOLOGY - Technical centers are bastions
of \*Qpure\*U technology research,
which is to say universities with massive defense department grants.
They turn construction materials
into technological advances thereby raising the technology
level of the country (which affects gun ranges, pollution, etc.).
.s1
RESEARCH - The research lab is a bastion of \*Qpure\*U medical research,
which is to say it is a large university
with massive March-of-Dimes funding.
The research lab turns construction materials into medical
discoveries which raise the research level of the country and
help retard the spread of disease (usually caused by the pollution
from technical centers).
.s1
NUCLEAR - The nuclear lab is a bastion of applied technology,
which is to say it is an immense underground building filled with
evil geniuses playing Adventure on huge computers.
You can build nuclear weapons here; see \*Qinfo build\*U for how.
.s1
LIBRARY/SCHOOL - The library/school sector is the foundation
of a country's educational structure.
It uses up construction materials to produce units of education
which raise the educational
level of the country, which in turn affects the efficiency of research
and
technology sectors.
.s1
ENLISTMENT - The enlistment sector is the boot camp of Empire.
It converts civilians into military.
Unlike the other production sectors,
it can use only military as workforce for that.
.s1
HEADQUARTERS - This is where you build land units; see \*Qinfo
build\*U for how.  Headquarters also repair incomplete or damaged land
units, provided there's enough work and materials.  They also give a
bonus to reaction radius for units deployed there,
allowing them to defend better.
.s1
Production takes place automatically at the update,
at a rate dependent on the efficiency of the sector,
your production efficiency for that product,
the presence of the required materials,
and the number of workers in the sector.  \*Qinfo Products\*U has
more.
.s1
The \*Qshow\*U command can be used to show special things about sector
types.  To check out build costs, you do:
.EX show sect build
.NF
sector type    cost to des    cost for 1% eff   lcms for 1%    hcms for 1%
-              0              0                 0              0
f              0              5                 0              1
other          0              1                 0              0

Infrastructure building - adding 1 point of efficiency costs:
       type          lcms    hcms    mobility    $$$$
road network            2       2           1       2
rail network            1       1           1       1
defense factor          1       1           1       1
.FI
.s1
Cost to des is the cost to designate the sector type. This is charged when you
give the designate command.
.s1
Cost for 1% efficiency is the $$ cost per point of efficiency built.
Lcms/hcms for 1% are similar.
.s1
Sectors that cost nothing to designate and just $1 per point of
efficiency are not shown.
.s1
The show command also shows infrastructure build costs.
Infrastructure is additional facilities you can build up in your
sectors.  See \*Qinfo Infrastructure\*U for details.
.s1
To check out various stats, you do:
.EX show sect stats
.NF
                        mob cost   max   max   naviga    packing   max
  sector type            0% 100%   off   def   bility      bonus   pop
\&. sea                    no way    0.00  0.00     sea     normal     0
^ mountain              2.4  1.2   1.00  4.00    land     normal   100
s sanctuary              no way    0.00 99.00    land     normal  1000
\e wasteland              no way    0.00 99.00    land     normal     0
- wilderness            0.4  0.4   1.00  2.00    land     normal  1000
c capital               0.4  0.2   1.00  2.00   canal     normal  1000
u uranium mine          0.4  0.2   1.00  2.00    land     normal  1000
p park                  0.4  0.2   1.00  1.50    land     normal  1000
d defense plant         0.4  0.2   1.00  1.50    land     normal  1000
i shell industry        0.4  0.2   1.00  1.50    land     normal  1000
m mine                  0.4  0.2   1.00  2.00    land     normal  1000
g gold mine             0.4  0.2   1.00  2.00    land     normal  1000
h harbor                0.4  0.2   1.00  1.50  harbor  warehouse  1000
w warehouse             0.4  0.2   1.00  1.50    land  warehouse  1000
* airfield              0.4  0.2   1.00  1.25    land     normal  1000
a agribusiness          0.4  0.2   1.00  1.50    land     normal  1000
o oil field             0.4  0.2   1.00  1.50    land     normal  1000
j light manufacturing   0.4  0.2   1.00  1.50    land     normal  1000
k heavy manufacturing   0.4  0.2   1.00  1.50    land     normal  1000
f fortress              0.4  0.2   2.00  4.00    land     normal  1000
t technical center      0.4  0.2   1.00  1.50    land     normal  1000
r research lab          0.4  0.2   1.00  1.50    land     normal  1000
n nuclear plant         0.4  0.2   1.00  2.00    land     normal  1000
l library/school        0.4  0.2   1.00  1.50    land     normal  1000
+ highway               0.4  0.0   1.00  1.00    land     normal  1000
) radar installation    0.4  0.2   1.00  1.50    land     normal  1000
! headquarters          0.4  0.2   1.00  1.50    land     normal  1000
# bridge head           0.4  0.0   1.00  1.00    land     normal  1000
= bridge span           0.4  0.0   1.00  1.00  bridge     normal   100
b bank                  0.4  0.2   1.00  2.25    land       bank  1000
% refinery              0.4  0.2   1.00  1.50    land     normal  1000
e enlistment center     0.4  0.2   1.00  2.00    land     normal  1000
~ plains                0.4  0.2   1.00  1.50    land     normal   100
@ bridge tower          0.4  0.0   1.00  1.50    land     normal   100
.FI
The two mob cost columns show the relative cost to move through such
sectors at 0% and 100% efficiency.  It changes linearly between 0% and
100%.
.s1
Combat strength is multiplied by sector offensive factor on attack and
defensive factor on defense.  Columns max off and def show them for
100% efficiency.  They're always 1 for 0%, except for mountains, where
the 0% defensive factor is 2.  Both factors change linearly between 0%
and 100%.
.s1
Some sector types are more adept at moving certain commodities: the
mobility cost is divided by a packing bonus.
.s1
Column max pop shows how many civilians and uncompensated workers the
sector type can hold.
.s1
Finally, to check out what sectors can do, use:
.EX show sect capabilities
.NF
  sector type             product  p.e.
^ mountain                dust      75%
u uranium mine            rad      100%
p park                    happy    100%
d defense plant           guns     100%
i shell industry          shells   100%
m mine                    iron     100%
g gold mine               dust     100%
a agribusiness            food     900%
o oil field               oil      100%
j light manufacturing     lcm      100%
k heavy manufacturing     hcm      100%
t technical center        tech     100%
r research lab            medical  100%
l library/school          edu      100%
b bank                    bars     100%
% refinery                petrol  1000%
.FI
Column product shows what the sector produces.  Column p.e. shows the
sector type's production efficiency.  Use command \*Qshow product\*U
to find out more about products, and see \*Qinfo Products\*U.
.s1
.s1
.SA "designate, show, improve, BTU, Sectors, Producing, Products, Transportation"
