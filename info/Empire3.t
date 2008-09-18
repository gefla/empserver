.TH Server "Empire3 Changes"
.NA Empire3 "Changes from the Empire2 server to the Empire3 Server"
.LV Expert
.s1
There have been several changes/fixes made to the Empire2 code in the
new Empire3 Server.  This outlines the various changes and how
they will affect you, the player.  Most of these changes were coded by
Ken Stevens <children@empire.net> and any bug-reports in the new
server should be directed to him.
.s1
Sectors
.in 0.6in
.L -
The delivery and distribution stuff was removed from the output of
"commodity" and each column was widened to 5 characters (to accomodate
the new system where sectors are now allowed to hold up to 9999 of
stuff)
.L -
Mountains will now mine gold at 3/4 the rate that gold mines do (thar's
gold 'n them thar hills!)
.L -
Mountains can now only hold 1/10th the population of normal sectors.
.L -
The mobility cost to move into a sector has been changed from:
.nf
    cost = (mcost - eff) / 5
  to:
    if (mcost < 25)
      cost = (2 + (3 - mcost) * eff) / 5
    else
      cost = (mcost - 20 * eff) / 5

OLD SYSTEM: eff   0%  100%     NEW SYSTEM: eff   0%  100%
              +  0.2   0.0                   +  0.4   0.0
              m  0.4   0.2                   m  0.4   0.2
              ^  5.0   4.8                   ^  5.0   1.0
.fi
Note that this means land units can attack 100% mountains now, so
there will be a tradeoff for the defender--either they can have a low
efficiency mountain that costs more mobility to attack but gets a
lower defense bonus, or they can have a high efficiency mountain that
costs less mobility to attack but gets a higher defense bonus.
.L -
New conditional query "coastal" which gets set when you take a
"census" of the sector.  Type "cens * ?coast=1" to get a list of all
of your sectors which are adjacent to water (and thus vulnerable to
assault).
.\" -----------------------------------------------------------
Ships
.in 0.6in
.L -
- The size of "ts1 trade ship 1" was reduced to 100 lcm, 50 hcm, $1500
(from 200 lcm, 100 hcm, $2500).  Most players were waiting for ts2
so this is a nice way to deal with that.
.in 0
.\" -----------------------------------------------------------
Planes
.in 0.6in
.L -
In the old system, planes would be charged either 10 mobility
(interceptors), 12 mobility (escorts) or 20 mobility (the rest) per
flight.  This cost was then multiplied by (100 / eff) with the
restriction that plane mobility could not go below -32.  Planes flying
missions were charged only 1/4 of this.  In the new system, planes are
charged a base rate of 5 mobility to prepare for takeoff.  On top of
this, they are charged a flight cost which is either 10 (interceptors
and escorts) or 20 (the rest), however this flight cost is scaled down
according to the length of the flight--it is multiplied by (flight
length) / (max plane range).  The flight cost is then scaled by
(100/eff) with the restriction that a plane can't go below -32
mobility.  Planes flying missions are charged 1/2 of this.
.L -
The mobility of a plane will now go down by one point for every two
points of damage it took from air-to-air combat, to a limit of -32
mob.  This is to simulate the high mobility cost of air-to-air combat,
and the fact that air-to-air combat is more effort for low tech planes.
.L -
SAM interception is now separate from plane interception; for each
attacking plane, if it cost at least $1000 to build, then one SAM will
be launched at it.  After SAMs have been fired, fighter planes will
intercept what's left over.
.L -
The restriction that high tech nations can not build low tech planes
has been removed (the above changes address the f1 SAM-soaking problem).
.L -
Marine missiles on interdiction now fire at a single ship rather
than the whole fleet.  They pick the ship which is the most valuable
according to (cost to build) * efficiency.  Furthermore, marine
missiles will only launch at a ship if the ship cost >= $1000 to build
or if it can carry land units or planes.
.L -
Sectors which your planes fly over when they fly missions will now be
added to your bmap.
.in 0
.\" -----------------------------------------------------------
Land Units
.in 0.6in
.L -
There are three new land units: 'cavalry 2', 'light infantry', and 'aau1'.  Here's the basic scheme.  The old cavalry are an average
between the somewhat weaker cav1 and somewhat stronger cav2.
light infantry are half the size of and slightly fastar than inf1's. 'aau1' come at the same tech as Zepplins and let low tech countries
try to throw rocks at low tech planes...
.in 0
.\" -----------------------------------------------------------
Maps
.in 0.6in
.L -
You now have 50 realms.
.L -
Each country now has two bmaps, their 'working' bmap and their 'true' bmap.  The true bmap contains only information put there by the
server.  The working bmap also contains information put there by the
bdes and sharebmap commands.  The 'bmap' command usually displays the
working bmap unless the player specifies 'bmap <SECTS> true'.
If the players working bmap becomes corrupted somehow, then they can
type 'bmap <SECTS> revert' to revert their working bmap back to their
true bmap.  This change was necessary to prevent abuse of the new 'sharebmap' command (which compares the players true bmaps when
determining whether they overlap).
.L -
There is a new command "sharebmap <COUN> <SECTS>".  If <COUN>
is friendly towards you and your 'true' bmaps already share enough information
in common, then your bmap will be added to their bmap.  For more details
see 'info sharebmap'.
.in 0
.\" -----------------------------------------------------------
Information
.in 0.6in
.L -
The new info page "info Mobility" gives the formulas for all
mobility costs in the game.
.in 0
.\" -----------------------------------------------------------
Communication
.in 0.6in
.L -
If someone else was the last person to use your country, then when you
login you will get a message like:
.NF
Last connection from: Mon Nov 13 04:48:04 1995
                  to: Mon Nov 13 05:13:12 1995
                  by: children@RedDragon.Empire.Net
.FI
.L -
If the game is a blitz, then the 'players' command will list other
countries which are logged on.  This will cut down on the 'is anybody
else logged on?' announcements.  Note that for players, 'last command'
and 'user@hostname' are not printed (only deities get to see this stuff).
.L -
You can now read old announcements which you've already thrown away by
typing:
.SY "wire <number of days>"
This works much in the same way as news.  The server automatically
deletes all announcements older than 7 days.
.L -
New server commands 'toggle sync', 'toggle async', and 'sync' permit
client-server database synchronization.
.L -
Before you type a telegram, you will now see:
.nf
    Enter telegram for Fodderland
    undo last line with ~u, print with ~p, abort with ~q, end with ^D or .
    1024 left:
.fi
  The new tilde escapes work very nicely.

.\" -----------------------------------------------------------
.in 0
.s1
Options
.nf
The following options have been made standard:
NEWPAF:         Plane combat is in nice easy-to-read columns
.fi
.s1
.SA "toggle, sharebmap, Clients, Server"
