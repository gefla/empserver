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
 COMMUNICATIONS           a  agribusiness           FINANCIAL
 +  highway               o  oil field	            b  bank
 )  radar installation    j  light manufacturing    
 #  bridge head           k  heavy manufacturing
 =  bridge span           %  refinery            
 @  bridge tower
.FI
.s1
.s1
.ce
BASICS
.s1
SEA - Sea sectors form natural barriers that can
only be crossed by ships, (made in harbors, below),
or spanned by bridges, (also below).
You can not designate anything else to be sea
nor can sea be designated to be anything else.
.s1
MOUNTAIN - Mountain sectors form another natural
barrier that cannot be redesignated; however, they can be
moved through (at great expense in terms of mobility units).
In an emergency, they can be used as capital. See \*Qinfo capital\*U
for more information.  In addition, mountains can only hold a maximum
of 1/10th the civilians and uncompensated workers that other sectors
can normally have.  And military, civilians and uncompensated workers that
will actually produce anything there are also limited to 1/10th.
.s1
SANCTUARY - Sanctuary sectors are created when
a new nation is created.
They are inviolate in that no one can fire at them or attack them.
This protection ends
when the new country first moves out of the sanctuary
(called \*Qbreaking sanctuary\*U); the sector then becomes a capital.
NOTE: in games with multiple sanctuaries per country ALL sanctuaries
become capitals when any one does!
.s1
WASTELAND - This is the result of the explosion of a nuclear device.
Wastelands are uninhabitable forever.
.s1
WILDERNESS - Most of the world is wilderness
at the beginning of the game.
Wilderness has no particular attributes;
you can move into it if unoccupied, thereby
making it your territory, but will probably want to
designate it as something else once you own it.
.s1
PLAINS - Plains sectors form another natural barrier that cannot
be redesignated.  They can be occupied and moved through however.
In addition, plains can only hold a maximum of 1/20th the civilians
and uncompensated workers that other sectors can normally have.
And military, civilians and uncompensated workers that
will actually produce anything there are also limited to 1/20th.
.s1
CAPITAL - Capitals are the source of bureaucracy time units (BTU's).
They accrue in proportion to the efficiency
of the capital and the number of civilians at work in it.
(see info BTU).
Most commands use up BTU's,
(see \*Qlist of commands\*U for numbers of BTU's used per command).
A nation may only have one active capital at a time
(although many sectors may be designated as capitals).
The capture of a capital will result in the loss of money from
the victim nation (see info capital),
Capitals are twice as efficient at defending
against attack as other sectors (except fortresses).
Use the capital command to make a capital the active one.
In a pinch, mountains can be used as capitals. They generate
very few BTU's, and represent being 'holed up in the mountains',
perhaps like Pancho Villa.  Note that if the BIG_CITY option is
enabled, then 'c' sectors stand for 'cities' and can hold 10x as many
civs.  They also get a 10x packing bonus for moving civs, and require
a certain number of lcm's, hcm's, and $$ to build (see show sect build).
.s1
PARK - Parks are provided solely for the convenience of the 
people.
Parks require construction materials to provide maintenance 
services and
add to a nation's \*Qhappiness\*U level.
The conversion of \*Qraw\*U materials takes place automatically
at a rate dependent on the efficiency of the sector,
the presence of the required materials,
and the number of workers in the sector.
.s1
.s1
.ce
COMMUNICATIONS
.s1
HIGHWAY - Whenever you move civilians, ore, gold,
etc, mobility units (\*Qmob\*U on the census) are consumed dependent
on how far and how much you move.
However if the movement is through 100% efficient highway
it costs much less to move.
.s1
RADAR - Radar stations can scan the surrounding
area (up to 16 units away for 100% efficiency) and generate
a radar plot identifying sector types at distances up to 1/3
their range and ships up to their full range.
.s1
BRIDGE HEAD - Bridge heads are the land based ends of bridges.
They, like harbors, turn construction materials into
bridge spans; see \*Qinfo build\*U for the details of this process.
and are much like highways except for three things:
.br
(1) Bridge spans provide food through fishing,
(the fertility can be thought of as fish-count).
(This is deity-settable, however.. in many games, bridges do not
provide food. Be sure to ask)
.br
(2) Bridge spans must maintain at least 20% efficiency or else they
collapse.
.br
(3) If the only bridge head supporting a particular bridge span is
redesignated as something else the bridge span will collapse.
(If the EASY_BRIDGES option is in use, bridges do not need bridgeheads.
Check \*Qinfo build\*U for the details)
.s1
BRIDGE TOWER - Bridge towers are like bridge heads you can build
out in the ocean.  You can only build bridge towers from a bridge
span, and in open water (not adjacent to any land or other bridge
towers.)  From a bridge tower, you can build other bridge spans.
The BRIDGETOWERS option must be enabled for you to be able to
build bridge towers.
.br
Bridge towers must maintain at least 20% efficiency or else they
collapse.  Any bridges connected to the tower will collapse as
well, unless held up by another adjacent structure (bridge head,
bridge tower or land if EASY_BRIDGES is enabled.)
.s1
.s1
.ce
FINANCIAL
.s1
BANK - Banks are used for smelting and storing gold bars.  
They include a smelter to refine gold dust into gold bars.
While the bars are stored in the bank the busy little bankers 
invest them and return a profit on their use.
Banks are of the Fort Knox variety; they are more impervious to shelling
than any other sector and militia in them fight twice as hard
against attack as those in industries.
Banks are also particularly adept at moving gold bars around;
bars are moved and stored in groups of four thus fewer mobility units
are required to move a gold bar from a bank than anywhere else.
.s1
.ce
INDUSTRIES
.s1
DEFENSE - In defense plants construction materials are turned into guns.
The conversion of \*Qraw\*U materials into guns
takes place automatically at a rate dependent on
the efficiency of the sector,
the presence of the required materials,
the technology level of the country,
and the number of workers in the sector.
See \*Qinfo Products\*U for information on the materials required to
produce guns.
.s1
SHELL INDUSTRY - These sectors are similar to defense
plant sectors except they turn construction materials into
shells.
The conversion of \*Qraw\*U materials takes place automatically
at a rate dependent on the efficiency of the sector,
the presence of the required materials,
the technology level of the country,
and the number of workers in the sector.
See \*Qinfo Products\*U for information on the materials required.
One shell is used each time you fire,
(except for submarines which use three shells to make one torpedo).
.s1
MINE - Mines produce iron ore by digging it out of the ground.
The rate at which it is produced is dependent on three factors;
the efficiency of the mine,
the number of civilians working in the mine,
and the mineral sample for the sector (\*Qmin\*U on the census report).
Iron is a \*Qrenewable\*U resource; i.e. digging up iron does not
deplete the mineral content of the sector.
.s1
GOLD MINE - Gold mines are similar to ordinary
mines, (above), except that they produce gold dust.
The rate at which it is extracted is dependent on three factors;
the efficiency of the sector,
the size of the labor force in the sector,
and the gold sample for the sector (\*Qgmin\*U on the census report).
The raw gold dust can be transported to any sector but if it is left
in a bank sector it will be refined into bars automatically .
Gold is a non-renewable resource; i.e. extracting the gold depletes
the gold content of the sector.
.s1
URANIUM MINE - Uranium mines are very similar to gold
mines, (above), except that they produce radioactive materials.
The rate at which it is extracted is dependent on three factors;
the efficiency of the sector,
the size of the labor force in the sector,
and the uranium content of the sector
(\*Quran\*U on the census report).
Uranium is a non-renewable resource; i.e. extracting it depletes
the content of the sector.
.s1
HARBOR - Harbors combine shipyard facilities and docks.
Construction materials are converted into ships
and the ships can be loaded and unloaded in the harbor,
(see \*Qinfo load\*U).
No ships are constructed until the \*Qbuild\*U command is given,
(see \*Qinfo build\*U).
.s1
WAREHOUSE - Warehouses are used to store shells,
guns, iron, gold dust, food, oil, light construction materials, and
heavy construction materials.  Moving the above out of a 
warehouse only takes one tenth the mobility cost it normally would.
Note: warehouses are often used as distribution centers.
.s1
AIRFIELD - Airplanes can only be built in airports and most can
only take off and land at airports.  (though some can take off
and land in other sectors and some can take of and land on
aircraft carriers see Ship-types for details) 
.s1
AGRIBUSINESS - These sectors are large farms and provide food.
Agribusiness sectors produce as much as five times as much food
as other sectors.
The harvesting takes place automatically
at a rate dependent on the efficiency of the sector,
the fertility of the sector (\*Qfert\*U in the census),
the technology level of the country,
and the number of workers in the sector.
See \*Qinfo Products\*U for details.
.s1
OIL - Oil fields produce oil by sucking it out of the ground.
The extraction of oil takes place automatically
at a rate dependent on the efficiency of the sector,
the oil content of the sector (\*Qoil\*U on the census report),
the technology level of the country,
and the number of workers in the sector.
Oil is a \*Qnon-renewable\*U resource; i.e. extracting oil
depletes the oil content of the sector.
.s1 
REFINERY - Refineries produce petroleum by processing oil.  
This production takes place automatically
at a rate dependent on the efficiency of the sector,
the number of workers in the sector, 
and  the technology level of the country.
.s1
LIGHT - Light manufacturing plants produce light construction materials
from iron.
This production takes place automatically
at a rate dependent on the efficiency of the sector,
the iron in the sector,
the technology level of the country,
and the number of workers in the sector.
.s1
HEAVY - Heavy manufacturing plants produce heavy construction materials
from iron.
This production takes place automatically
at a rate dependent on the efficiency of the sector,
the iron in the sector,
the technology level of the country,
and the number of workers in the sector.
.s1
.s1
.ce
MILITARY / SCIENTIFIC
.s1
FORTRESS - Fortress sectors have many special characteristics;
you can fire guns from fortresses,
whenever an attack is launched from a fort or on
a fort the militia in the fort are stronger than military
in any other type of sector by an amount proportional to
the efficiency of the fort. (See information below in sector
stats)
Also, forts and commodities in forts are MUCH more resistant
to damage than normal sectors.
.s1
TECHNOLOGY - Technical centers are bastions
of \*Qpure\*U technology research,
which is to say universities with massive defense department grants.
They turn construction materials
into technological advances thereby raising the technology
level of the country (which affects gun ranges, pollution, etc.).
The conversion of \*Qraw\*U materials takes place automatically
at a rate dependent on the efficiency of the sector,
the presence of the required materials,
the education level of the country,
and the number of workers in the sector.
.s1
RESEARCH - The research lab is a bastion of \*Qpure\*U medical research,
which is to say it is a large university
with massive March-of-Dimes funding.
The research lab turns construction materials into medical
discoveries which raise the research level of the country and
help retard the spread of disease, (usually caused by the pollution
from technical centers).
The conversion of \*Qraw\*U materials takes place automatically
at a rate dependent on the efficiency of the sector,
the presence of the required materials,
the education level of the country,
and the number of workers in the sector.
.s1
NUCLEAR - The nuclear lab is a bastion of applied technology,
which is to say it is an immense underground building filled with
evil geniuses playing Adventure on huge computers.
The nuclear lab turns construction materials into nuclear weapons.
No devices are constructed until the \*Qbuild\*U command is given,
(see \*Qinfo build\*U).
.s1
LIBRARY/SCHOOL - The library/school sector is the foundation
of a country's educational structure.
They use up construction materials to produce units of education
which raise the educational
level of the country (which affects the efficiency of research and
technology sectors).
The conversion of \*Qraw\*U materials takes place automatically
at a rate dependent on the efficiency of the sector,
the presence of the required materials,
and the number of workers in the sector.
.s1
ENLISTMENT - The enlistment sector is the boot camp of Empire.
It converts civilians into militia, once the efficiency level
has been raised to 60%.
Unlike the other production sectors,
it uses only militia as a workforce,
and converts civilians in the sector into military.
.s1
HEADQUARTERS - This is where basic militia are converted into units.
Headquarters also give a bonus to reaction radius for units in them,
allowing them to defend better.
.s1
The \*Qshow\*U command can be used to show special things about sector
types. For example:
.EX show sect build
.NF
sector type    cost to des    cost for 1% eff   lcms for 1%    hcms for 1%
f              500            5                 0              1

Infrastructure building - adding 1 point of efficiency costs:
       type          lcms    hcms    mobility    $$$$
road network            1       1           1       1
rail network            2       2           1       3
defense factor          1       1           1       1
.FI
.s1
Cost to des is the cost to designate the sector type. This is charged when you
give the designate command.  Note that if the BIG_CITY option is
enabled, then 'c' sectors also cost money, lcm's and hcm's to build,
and they can hold 10x as many civs as other sectors.
.s1
You can also build infrastructure into your sectors.  That is how the
mobilty, for both commodities and units, is determined.  You can also
raise the defensive value of your sectors to a maximum amount (which is
shown in the "show sect stats" described below.)
.s1
.EX des 2,0 f
That just cost you $500
.s1
Cost for 1% efficiency is the $$ cost per point of efficiency built.
Lcms/Hcms for 1% are similar.

.EX show sect stats
.NF
                                 max   max   --  packing bonus  --
  sector type           mcost    off   def   mil  uw civ bar other
\&. sea                       0   0.00  0.00     1   1   1   1     1
^ mountain                 25   1.00  4.00     1   1   1   1     1
s sanctuary                 0   0.00 99.00     1   1   1   1     1
\e wasteland                 0   0.00 99.00     1   1   1   1     1
- wilderness                2   1.00  2.00     1   1   1   1     1
c capital                   2   1.00  2.00     1   1   1   1     1
u uranium mine              2   1.00  2.00     1   1   1   1     1
p park                      2   1.00  1.50     1   1   1   1     1
d defense plant             2   1.00  1.50     1   1   1   1     1
.FI
This shows the relative cost to move through the sector types, and the
offensive and defensive values of the sector types. For example, a capital
with a 100% defensive efficiency is twice as good as a park (since the
base value is 1.00.)  All sectors start with a 0% defensive efficiency,
thus they all start with a defense of 1.00.  You can increase the
defensive capability of any sector up to the amount shown by the "max def"
column.

.EX show sect capabilities
.NF
                                                 --- level ---          reso 
  sector type             product use1 use2 use3 level min lag eff% $$$ dep c
^ mountain                dust                           0   0   75   0  20 d
u uranium mine            rad                    tech   40  10  100   2  35 r
p park                    happy    1 l                   0   0  100   9   0  
d defense plant           guns     1 o  5 l 10 h tech   20  10  100  30   0 g
i shell industry          shells   2 l  1 h      tech   20  10  100   3   0 s
m mine                    iron                           0   0  100   0   0 i
g gold mine               dust                           0   0  100   0  20 d
a agribusiness            food                   tech  -10  10  900   0   0 f
o oil field               oil                    tech  -10  10  100   0  10 o
j light manufacturing     lcm      1 i           tech  -10  10  100   0   0 l
k heavy manufacturing     hcm      2 i           tech  -10  10  100   0   0 h
t technical center        tech     1 d  5 o 10 l edu     5  10  100 300   0  
r research lab            medical  1 d  5 o 10 l edu     5  10  100  90   0  
l library/school          edu      1 l                   0   0  100   9   0  
b bank                    bars     5 d                   0   0  100  10   0 b
% refinery                petrol   1 o           tech   20  10 1000   1   0 p
.FI
This shows what the sector produces, raw materials required to make
one product, tech and edu requirements, production efficiency
multiplier, monetary cost and resource depletion.  See info
\*QProducts\*U for details.
.s1
.SA "improve, designate, show, BTU, Sectors"
