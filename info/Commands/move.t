.TH Command MOVE
.NA move "Transport ore, civilians, guns, etc"
.LV Basic
.SY "move <ITEM> <SECT> <NUMBER> <ROUTE|DESTINATION>"
The move command is crucial to Empire; it carries out the movement
of civilians, military, ore, guns, shells, food, planes, etc. on land.
Note that you can only move into sectors that you own, so you cannot
take sectors via move.  To take sectors, use the "explore" command,
which uses the same syntax as move.
.s1
<ITEM> can be any one of:
.NF
c       civilians
m       military
u	uncompensated workers
f       food
g       guns
s       shells
p       petrol
i       iron ore
d       gold dust
b       bars of gold
l       light construction materials
h       heavy construction materials
r	radioactive materials
.FI
.s1
If <SECT> is not provided in the command line,
the program will request the starting sector
then display the number of mobility units in that sector,
its designation, and its coordinates in the form:
.s1
.NF
<97.0: a -6,4>
.FI
.s1
which indicates 97 mobility units in agribusiness sector -6,4.
You may respond with any combination of:
.s1
.NF
y  for up-left
u  for up-right               y    u
g  for left                    \e  /
j  for right               g  --  --  j
b  for down-left               /  \e
n  for down-right             b    n
v  for view
m  for map
h  for end of movement
.FI
.s1
The \*Qview\*U command shows the sector efficiency
and some of the contents of the sector you are currently occupying.
.s1
Since the <ROUTE> can be specified in the command line,
simple movements are often typed as a single line, such as:
.EX move m 2,0 20 jjjh
Or some movements may be done partly on the command line and partly
interactively:
.EX move c 2,2 18 jj
.NF
<32.3: g 6,2> v
10% gold mine with 3 civilians.
<32.3: 6,2> h
.FI
.s1
You may also simply specify the destination sector.
In this case,
empire will set the path to be the cheapest path (in terms of
mobility) from the current sector to the destination sector,
if such a path exists.
.s1
For example,
.EX move c 2,2 18 6,2
.s1
.L "Military control"
.s1
In order to move something (other than mil) out of a sector, you must have
military control of the sector. Military control is defined as having mil in
the sector equal to at least 1/10th the number of unconverted civilians there.
Units count as if they were straight mil, i.e. a 100% unit that contained 50
mil would count as 50 mil for the purposes of control)
.s1
.L Mobility
See 'info Mobility' for a description of how much mobility it costs to
move something.
.s1
.L Mines
Note that while moving, you might hit mines. The more stuff you're
moving, the greater the chance of setting off any
mines in the sectors you move through. See info lmine for more details.
.s1
.L Interdiction
Any enemy units on interdiction mission may be triggered by the move
(the more you move, the bigger the chance they interdict).  The
interdiction will damage the goods you are moving, and will also cause
some damage to the sector you were moving into.
.s1
.L "Unhappy civilians"
.s1
Also, when moving unhappy civilians (work percentage less than 100%), 
unhappiness is contagious. The work percentage of the
destination sector is computed by averaging the incoming civ's happiness
with the happiness of the civs already there. For example, say you move
100 civs with work percentage of 0% into a sector with 100 fully happy
civilians (100% work). The work would be:
.ti 3
((migrants * their work) + (people at dest * their work) / (total civs)
((100 * 0%) + (100 * 100%)) / (100+100) =  100/200 = 50%
.s1
.L "Plague"
If a sector is infected with the plague, then anything moved out of
the sector will infect the sector that it moves into.  Note that
sectors moved through will not catch the plague.
.s1
.SA "navigate, transport, test, explore, Plague, Mobility, Commodities, Moving"
