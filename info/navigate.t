.TH Command NAVIGATE
.NA navigate "Move ship or fleet around"
.LV Basic
.SY "navigate <SHIP/FLEET> <ROUTE|DESTINATION>"
The navigate command is the \*Qmove\*U command applied to the sea.
You can control one ship or an entire fleet with it, but they must all
start in the same sector.
.s1
A ship must have at least one crew
(which may be civilian or military,
but not an uncompensated worker)
to be navigated.
When you navigate, you see the hex of sectors around the flagship.
.s1
The program will prompt with the maximum and minimum mobility of the
ships in the fleet, and its current position in the form:
.NF
<57.0:23.5: -6,4>
.FI
which means the ships have between 23.5 and 57 mobility units, and
are in sector -6,4.
You may indicate the direction you would like the fleet to move
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
f  to change flagship
i  to list ships you are navigating
v  to view oil content and fish
r  for radar
l  to look at land and nearby ships
s  for sonar
m  to sweep mines
d  to drop mines
h  for end of movement
.FI
.s1
.s1
The \*Qradar\*U command will cause the flagship to use it's radar, if any.
You may also give a ship number or fleet or group of ships on the same
line: \*Qr 12/13/14/73\*U
.s1
The \*Qlookout\*U and \*Qsonar\*U commands work in the same way as radar.
.s1
The \*Qmap\*U command will give you a
map surrounding the current position.
By default, it will be around the flagship.
You can also supply additional parameters:
.EX <32.3: g 6,2>M 3 ls
.s1
is equivalent to a \*Qsmap\*U command. like
this:
.EX map 3 ls
The \*Qbmap\*U command works in the same way, it is equivalent to a
\*Qsbmap\*U command.
.s1
The \*Qflagship\*U command will the change the flagship.
By default, it will select next ship in the list, but you may also
specify a flagship.
.s1
The \*Qdrop\*U command will drop mines using ships
with mine capability.
You can specify the ship number or fleet or groups of ships
and the number of sea mines on the same line: \*Qd 12/13 10\*U.
An omitted unit number defaults to the flagship,
for example \*Qd 10\*U drops ten sea mines from the flagship.
If you omit the number of sea mines as well, the flagship will drop one
mine.
.s1
The \*Qsweep\*U command will have the minesweepers in the navigating
group search for mines in the current sector.
This costs them the equivalent of 1 sector's mobility.
.s1
Note: the lookout, radar, sonar, drop and sweep commands use BTUs,
just as if you'd typed them
separately from the command line.
.s1
Since the <ROUTE> can be specified in the command line,
simple ship movements are often typed as a single line, such as:
.EX nav 19 jjjh
.s1
Or some movements may be done partly on the command line and partly
interactively:

.EX nav 18 jj
.NF
Flagship is pt   patrol boat (#18)
 . . 
. . .
 . . 
<112.7:112.7: -4,-2> v
pt   patrol boat (#18) @ -4,-2 0% sea
 . . 
. . .
 . . 
<112.7:112.7: -4,-2> gh
pt   patrol boat (#18) stopped at -6,-2
Path taken: jjg
.FI
.s1
You may also simply specify the destination sector.  In this case, the
ships will take a shortest known path to the destination sector.
.s1
For example,
.EX nav 18 -6,-2
.NF
Flagship is pt   patrol boat (#10)
 . . 
. . .
 . . 
<104.2:104.2: -6,-2> h
pt   patrol boat (#18) stopped at -6,-2
Path taken: jjg
.FI
.s1
The formula for the movement cost for 1 sector is:
.NF

	480 / (( speed + speed * (50+tech)/(200+tech) ) * Efficiency/100)

.FI
.s1
Moving a ship through a sector that has been mined risks hitting a
mine (see \*Qinfo Hitchance\*U).
Minesweepers can remove up to five mines per pass through a
sector (indicated by the message \*QSweep...\*U).
Minesweepers also take 1/2 normal damage from mines.
.s1
Also, note that some things may affect you while you are moving. Forts
belonging to hostile countries will fire at you when you come within
range.  Hostile land/sea/air units may fire at/bomb your ships,
if they're on interdiction missions (see \*Qinfo mission\*U).
The chance of bombs hitting your ships is determined by
the ship that is easiest to hit (see \*Qbomb\*U for the chance of
hitting).  Any damage incurred is divided evenly among
the navigating ships.
.s1
If your enemy has a
stack of missiles on interdiction mission, then they will
automatically fire one after another until all of your \*Qvaluable\*U
ships are sunk.
.s1
.L NOTE
.s1
While navigating, you cannot enter a sector that belongs to another
country unless they have FRIENDLY relations with you. This includes bridges, and harbors.
.s1
.SA "mine, Ship-types, mission, Ships, Transportation, Moving"
