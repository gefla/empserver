.TH Command NAVIGATE
.NA navigate "Move ship or fleet around"
.LV Basic
.SY "navigate <SHIP/FLEET> <ROUTE|DESTINATION>"
The navigate command is the \*Qmove\*U command applied to the sea.
You can control one ship or an entire fleet with it.
.s1
A ship must have at least one crew
(which may be civilian or military,
but not an uncompensated worker)
to be navigated.
When you navigate, you see the hex of sectors around the flagship.
(The lowest numbered ship is always considered the flagship).
.s1
The program will prompt with the maximum and minimum mobility of the
ships in the fleet, and the current sector of the flagship in the form:
.NF
<57.0:23.5: -6,4>
.FI
which means one ship has 57 mobility units,
some other ship in the fleet has 23.5 mobility units and
the flagship is in sector -6,4.
You may indicate the direction you would like the fleet to move
by typing a string of letters consisting of any combination of the
following:
.NF
y  for up-left
u  for up-right               y    u
g  for left                    \\\\  /
j  for right               g  --  --  j
b  for down-left               /  \\\\
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
h  for end of movement
.FI
.s1
While navigating you may also use radar, sonar, or lookout.
(by entering r, s, or l) By default it will radar, sonar, or 
lookout from the flagship. You may also
give a ship number or fleet or group of ships on the same
line: "r 12/13/14/73"
.s1
You may also use minesweepers in the currently navigating group to
sweep mines. This costs them the equivalent of 1 sector's mobility.
For example: "m 12/13"
.s1
Since the <ROUTE> can be specified in the command line,
simple ship movements are often typed as a single line, such as:
.EX nav 19 jjjh
.s1
Or some movements may be done partly on the command line and partly
interactively:

.EX nav 18 jj
.NF
<112.7:112.7: -4, -2> v
patrol boat #18 on open sea @ -3,-2
<112.7:112.7: -4, -2> gh
patrol boat #18 stopped at -6,-2
.FI
.s1
You may also simply specify the destination sector
on the command line. In this case,
empire will set the path to be the cheapest path (in terms of
length) that currently exists. The unit(s) will move to the
destination sector, and then ask for more input.
.s1
For example,
.EX nav 18 -6,-2
.NF
<104.2:104.2: -6, -2> h
patrol boat #18 stopped at -6,-2
.FI
.s1
Note that if you are naving multiple ships, you may only specify a
destination sector on the command line if all the ships start in the
same sector.
.s1
The formula for the movement cost for 1 sector is:
.NF

	480 / (( speed + speed * (50+tech)/(200+tech) ) * Efficiency/100)

.FI
.s1
Moving a ship through a sector that has been mined
introduces a chance of mines/(mines + 50) of hitting a mine.
Minesweeps can remove up to five mines per pass through a
sector (indicated by the message \*QSweep...\*U).
Minesweeps also take 1/2 normal damage from mines.
.s1
Also, note that some things may affect you while you are moving. Forts
belonging to hostile countries will fire at you when you come within
range. Hostile land/sea/air units may fire at/bomb your ships, if they're
on interdiction missions (see info mission).  If your enemy has a
stack of missiles on interdiction mission, then they will
automatically fire one after another until all of your ships are sunk.
The chance of missiles and planes hitting your ships is determined by
the "worst" ship in your fleet (see \*Qbomb\*U for the chance of
hitting).  Any damage incurred by the fleet is divided evenly among
all the ships in the fleet which are in the same sector.
.s1
.L NOTE
.s1
While navigating, you cannot enter a sector that belongs to another
country unless they have FRIENDLY relations with you. This includes bridges, and harbors.
.s1
.SA "mine, Ship-types, mission, order, sail, Ships, Moving"
