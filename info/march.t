.TH Command MARCH
.NA march "Move a land unit or units"
.LV Basic
.SY "march <LAND UNIT/ARMY> <ROUTE|DESTINATION>"
The march command is the \*Qmove\*U command applied to land units.
You can control one unit or an entire army with it, but they must all
start in the same sector.
.s1
Land units may only move into sectors you own, sectors of countries
you are allied with or deity-owned sectors with 1 exception.  Spies.  Spies
may move anywhere, with a chance of getting caught.  See \*Qinfo Spies\*U for
more information.
.s1
The program will prompt with the maximum and minimum mobility of the
land units in the army, and its current position in the form:
.NF
<57.0:23.5: -6,4>
.FI
which means the land units have between 23.5 and 57 mobility units, and
are in sector -6,4.
You may indicate the direction you would like the army to move
by typing a string of letters consisting of any combination of the
following:
.NF
y  for up-left
u  for up-right               y    u
g  for left                    \e  /
j  for right               g  --  --  j
b  for down-left               /  \e
n  for down-right             b    n
.FI
Other commands you may type while navigating are:
.NF
M  for map
B  for bmap
f  to change the leader
i  to list land units you are marching
r  for radar
l  for lookout
m  to sweep landmines
d  to drop landmines
h  for end of movement
.FI
.s1
The \*Qradar\*U command will cause the lead unit to use it's radar, if any.
You may also give a unit number or army or group of units on the same
line: \*Qr 12/13/14/73\*U
.s1
The lookout command works in the same way as radar.
.s1
The \*Qmap\*U command will give you a
map surrounding the current position.
By default, it will be around the leader.
You can also supply additional parameters:
.EX <32.3: g 6,2> M 3 ls
.s1
is equivalent to a \*Qlmap\*U command. like
this:
.EX lmap 3 ls
The \*Qbmap\*U command works in the same way, it is equivalent to a
\*Qlbmap\*U command.
.s1
The \*Qleader\*U command will the change the leader.
By default, it will select next unit in the list, but you may also
specify a land unit.
.s1
The \*Qdrop\*U command will lay land mines using engineers.
You can specify the unit number or army or groups of units
and the number of land mines on the same line: \*Qd 12/13 10\*U.
An omitted unit number defaults to the leader,
for example \*Qd 10\*U lays ten land mines from the leader.
If you omit the number of land mines as well, the leader will lay one
mine.
.s1
The \*Qsweep\*U command will have the engineers in the marching
group search for mines in the current sector.
This costs as much mobility as entering a sector with mobility cost
of 0.2.
.s1
Note: the lookout, radar, drop and sweep commands use BTUs,
just as if you'd typed them
separately from the command line.
.s1
Since the <ROUTE> can be specified in the command line,
simple unit movements are often typed as a single line, such as:
.EX march 19 jjjh
.s1
Or some movements may be done partly on the command line and partly
interactively:

.EX march 18 yy
.NF
Leader is cav  cavalry #0
 c . 
. + .
 . h 
<105.6:105.6: -24,-2> l
Your capital 100% efficient with 549 civ with 10 mil @ -5,-3
Your highway 100% efficient with 549 civ with 0 mil @ -4,-2
Your harbor 100% efficient with 549 civ with 109 mil @ -3,-1
 c . 
. + .
 . h 
<105.6:105.6: -4,-2> yh
cav  cavalry #0 stopped at -5,-3
.FI
.s1
You may also simply specify the destination sector.  In this case, the
land units will take a cheapest path (in terms of mobility) to the
destination sector.
.s1
For example,
.EX march 18 -6,-2
.NF
Leader is cav  cavalry #0
Using path 'yyh'
cav  cavalry #0 stopped at -6,-2
.FI
.s1
See \*Qinfo Mobility\*U for the mobility cost to march land units.
.s1
Land units entering a sector that has been mined may trip landmines
(see \*Qinfo Hitchance\*U).
Units with engineering capabilities can remove up to five mines per
pass through a sector (indicated by the message \*QSweep...\*U).
Such units also take 1/2 normal damage from mines.
See info lmine for more details.
.s1
Hostile land/sea/air units may fire at/bomb/missile your units, if
they're on interdiction missions (see \*Qinfo mission\*U).
The chance of missiles and bombs hitting your land units is determined by
the land unit that is easiest to hit (see \*Qbomb\*U for the chance of
hitting).  Any damage incurred is divided evenly among
the marching land units.  Collateral
damage will be done to the sector that the units were marching into.
.SA "Unit-types, lmine, LandUnits, Transportation, Spies, Moving"
