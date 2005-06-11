.TH Server "A Brief History of the Different Versions of Empire"
.NA History "Empire Revision History"
.LV Expert
.NF

1985 PSL Empire, Peter Langston

Naturally, the original creator (or at least the last surviving
implementor) of Empire is Peter Langston.  He distributed the
game that we all wanted to hack on, but naturally enough he
didn't give out the source!  (he wasn't dumb...he probably knew
what we'd do to his code)


1986 UCSD Empire 0.x, Dave Pare

Made Empire multi-player with fixed update times.  Added hex map, planes, nukes,
satellites, ships, market, distribute, and updates.

Jim Reuter (and his PDP/11 decompiler) provided Dave Pare with a large
quantity of amazingly readable decompiled code.  His contribution
came at a time when Dave's interest in reverse-engineering was
flagging, and it provided what was necessary to create the
original UCSD Empire.

David Muir Sharnoff was responsible for promoting the early
development of Empire 0.X.  Without David it is unlikely if Empire
would have escaped UCSD!  He took on the onerous task of managing
the source, creating mailing lists, applying patches, and generally
making the early versions of UCSD Empire releasable -- they
sure weren't when they left Dave Pare's hands...

The rest of the folks who contributed to the early versions did
so with code and/or ideas.  (If I've forgotten anyone, let me know!)
They appear in chronological order.

			Keith Muller
			James Anderson
			Julian Onions
			Lewis Jansen
			Peter Vukovich
			Jeff Anton
			Jeff Wallace
			Gregory Paris
			Edward Rynes
			Sean Casey
			Phill Everson
			Kurt Pires
			Jeff Okamoto
			Nick Lai
			Steven Grady
			Jim Griffith
			Chris Metcalf
			Steven Grimm
			Mike St. Johns


1986 BSD Empire 1.1, Dave Pare

The following people (listed in alphabetical order) have contributed
to the development of BSD Empire 1.1.  Without these people, BSD
Empire would never have happened.  Thanks also go to the XCF at the
University of California, Berkeley, for providing the facilities on
which we performed this herculean task.

			David Davis
			Steven Grady
			Jim Griffith
			Beorn Johnson
			Phil Lapsley
			Jonathan Lee
			Kurt Pires
			Jeff Wallace


198? UCB Empire 1.2, ??

Added announcements, food, removed weather, parks, petrol,
edu, terrorists, shoot, many new ships.


198? BSD Empire -- KSU Distribution (1.04), Jeff Bailey

One of the goals of the KSU team was to make the server as
configurable as possible.  As such, they added many OPTIONS, and
global constants.  Added abms, asats, bmap, autonav, scuttle,
NUKEFAILDETONATE, MISSINGMISSILES, SHIPNAMES, NEUTRON, RANGEEDIT,
MISSDEF, NOFOOD, UPDATESCHED, DEMANDUPDATE, ORBIT, FALLOUT, SAIL,
ALLYHARBOR, and fixed many bugs.  Other contributers were:
			Yaser Doleh
			Robert Forsman
			Doug Hay
			Alex Shatsky


198? Merc Empire ?.?, ??

Fixed many bugs and removed loans, added grind, starvation, and prod.


1992 Chainsaw Empire 1.0, Thomas Ruschak

Added land units, SUPER_BARS, EASY_BRIDGES, SLOW_WAR.


1992 Chainsaw Empire 2.0, Thomas Ruschak

Added trade ships, fuel, semi-land ships, ASW planes, payoff, wire, SNEAK_ATTACK
retreat paths, sweep planes, budget.  Tom thanks the following people
for helping him with ideas and play-testing:

			Jorge Diaz (Ansalon)
			Tom Tedrick (Afrika Korps)
			Keith Graham (DreamLands),
			Dave Nye (Evil_Empire)
			Sasha Mikheev (Dolgopa)
			Baldric
			Elsinore
			Brett Reid (Resvon)
			Sam Tetherow (Kazzad'ur)
			Tharkadia
			Yikes


1993 Chainsaw Empire 3.0, Thomas Ruschak

Added land units, missions, bestpath, people take less damage from shelling, +1
range bonus for 60% forts, cede, neweff, starva, forts interdict
ships, mountain caps, RES_POP, NEW_STARVE, NEW_WORK, uncrewed ship
decay, anonymous sub-launched missiles, stop & start, bdes.


1995 Empire 2.0, Dave Pare

emp_player, emp_tm, and emp_update, were consolidated into one program
called emp_server.  A threads package called "lwp threads" was used to
manage the player threads.  Many options were made standard.  Kevin Klemmick
added HIDDEN and NEWPAF options.  Many options were made standard.
These are the people who submitted patches for Empire 2.0:
			Chad Zabel (3 letter abbrev's, & autonav)
			Ken Huisman
			Scott Ferguson (Linux port)
			Kevin Klemmick
			Doug Hay (threads debugging)
			Bill Canning (AIX port)
			Julian Onions


1995 Empire 2.1 beta, Ken Stevens

Organized info pages, wrote a "configure" script, made minor
improvements to many commands, rewrote missile, navigation, and march
code.  Consolidated launch/bomb and missile/plane interdiction so that
hit chance and damage is the same whether it's a mission or done "by
hand".  Added collateral damage, interdiction nuke detonation,
"friendly" trade relations, BIG_CITY, ATT_INTERDICT, 
Consolidated sail, navi, and order.  Added toggle, flash, wall,
shutdown, strength.  Other contributers:
			Chad Zabel (ship anti-missile defense)
			Julian Onions (runtime configuration)
			Sasha Mikheev (Linux port)

1995 Empire 2.2 beta, Ken Stevens

Completely rewrote attack, assault, board, and paradrop.  Added
"players", "skywatch", "disarm", tend land units.  Other contributers:
			Craig Leres
			Janjaap van Velthooven (IRIX port)
			Ken Huisman (RCS source managemant)


1995 Empire 2.3, Ken Stevens

After running a series of play-test games, Ken fixed all known bugs in
the server, organized and rewrote a lot of documentation (including
this info page) and released the Empire2 server out of beta.

1995 Empire 3.0, Ken Stevens

Empire 3.0 implemented the C_SYNC RFC, a powerful platform independent
client-server protocol for synchronizing the client database with the
server database.  The Empire 3.0 server was released with the Empire
Toolkit written by Kevin Morgan, a portable C library which parses the
C_SYNC messages from the server into a database for the client.  Thus,
clients can link with the Empire Toolkit and be confident that when
the server gets upgraded, their client will still work.  Note that the
C_SYNC protocol is asynchronous so, in particular, players will be
able to watch their neighbors sail ships past their coast etc...

1996 Empire 4.0, Wolfpack (http://www.wolfpackempire.com)

A group of people got together to form a new server project.  This project
is a new project, and is a complete takeoff from the Empire 3.0 server
project.  There are many additions and some subtractions from the 3.0
code base.  The Wolfpack is headed up by Steve McClure and consists of the
following people:

                        Mark Ballinger
                        Geoff Cashman
                        Bill Frolick
                        William S. Kaster
                        Pat Loney
                        Steve McClure
                        Richard Moss
                        Walter Smith
                        Sam Tetherow
                        Yannick Trembley
                        John Yockey

1998 Empire 4.2, Wolfpack (http://www.wolfpackempire.com)

The above authors (Dave Pare, Jeff Bailey, Thomas Ruschack, Ken Stevens
and Steve McClure) agreed to re-release the source code under the GNU
GPL.  The source was already freely released to the public, but any
copyright issues (such as gaining permission from Phil Lapsley for the
A* code and removing any other copyrighted code which would violate
the GPL) were cleared up, and the licensing information was put in
place to keep the server source free forever.  Yee-haw!

In addition, the server was made run-time configurable (yes, including
WORLD_X and WORLD_Y) so that you didn't have to re-build an entire server
each time you changed an option (you just have to rebuild the world 
sometimes. ;-) )

The source is still managed by the Wolfpack team.

.FI
.SA "Server"
