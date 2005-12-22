.TH Command EXPLORE
.NA explore "Claim unowned land"
.LV Basic
.SY "explore <ITEM> <SECT> <NUMBER> <ROUTE|DESTINATION>"
The explore command takes over part of the functionality of move.
Explore is used to stake your claim to unowned sectors.
Only civilians and military can be used to explore.
.s1
<ITEM> can be any one of:
.NF
c       civilians
m       military
.FI
.s1
If <SECT> is not provided in the command line,
the program will request the starting sector
then display the resources of that sector and a map of the
adjoining sectors.
The prompt consists of the number of mobility units in that sector,
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
.EX explore m 2,0 20 jjjh
Or some movements may be done partly on the command line and partly
interactively:
.EX explore c 2,0 1 gg
.NF
    - ^         min gold fert  oil uran
   - - +         67    0    0    0   82
    - ^
<125.4: - -4,0> v
0% wilderness with 0 civilians.
    - ^         min gold fert  oil uran
   - - +         67    0    0    0   82
    - ^
<125.4: - -4,0> h
Sector -4,0 is now yours.
.FI
.s1
You may also simply specify the destination sector on the
command line. In this case,
empire will set the path to be the cheapest path (in terms of
mobility) from the current sector to the destination sector,
if such a path exists. However, the best path cannot extend out
of your owned sectors, so the best use for this feature is to use
it to get to the edge of your owned territory, and then explore
manually from there.
.s1
For example,
.EX explore 2,0 1 -2,0
.NF
    ^ -         min gold fert  oil uran
   - + c         67    0    0    0   82
    ^ -
<126.2: - -2,0> g
    - ^         min gold fert  oil uran
   - - +         67    0    0    0   82
    - ^
<125.4: - -4,0> h
Sector -4,0 is now yours.
.FI
.s1
See 'info Mobility' for a description of how much mobility it costs to
explore into sectors.
.s1
Note that while exploring, you might hit mines. The more civs or
mil you explore with, the greater the chance of setting off any
mines in the sectors you move through. See info lmine for details.
.s1
.SA "move, test, navigate, transport, lmine, Populace, Sectors"
