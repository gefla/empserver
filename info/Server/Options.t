.TH Server OPTIONS
.NA Options "Server options"
.LV Expert
.nf
The following options are from KSU code:

SHOWPLANE:	planes/units on ships/units up for trade are shown
PINPOINT(etc):	Missile exist which target ships instead of sectors
SAIL:		another way to automatically move ships (may also be buggy)
NUKEFAIL(etc):	nukes on missiles that explode in launching may detonate
SHIPNAMES:	ships may be named
DEMANDUPDATE:	updates may be allowed on player demand
UPDATESCHED:	updates can be controlled by the 'hours' file. NITP

The following options were introduced in the Chainsaw server:

EASY_BRIDGES:	bridges can be built from any sector
SUPER_BARS:	gold bars aren't destroyed by shelling or bombing
ALL_BLEED:	you get tech bleed from all countries, not just allies
DRNUKE:		research is required to make nukes
SLOW_WAR:	War declaring takes time. No land attacks unless at war.
SNEAK_ATTACK:	If SLOW_WAR, allows you to pay money and sneak one attack.
NOMOBCOST:	ships pay 0 mob to fire. Subs pay 1/2 a sect's movement cost
TRADESHIPS:	you can build/nav/scuttle trade ships to make money
FUEL:		ships need fuel to gain mobility
RES_POP:	Research affects max sector population.
NOFOOD:		No food is required.
BLITZ:		Turns NOFOOD option on and players get infinite BTU's.
NONUKES:	No nuclear weapons
NO_PLAGUE:	Plague is disabled.
NEUTRON:	Enables neutron bombs
FALLOUT:	Sectors are damaged by radiation for a few updates after blast

The following options were introduced in the Empire2 Server:

HIDDEN:		Hides information about countries you haven't contacted
LOSE_CONTACT:	In HIDDEN, lose contact after a few updates
ORBIT:		Satellites can orbit the planet
BIG_CITY:	'c' sectors cost $, lcm, hcm to build and hold 10x civs
INTERDICT_ATT:	Interdict units & mil as they move in after an attack

The following options were introduced in the Empire4 Server:
BRIDGETOWERS:   You can build bridge towers, which allow you to build
                 bridges from them.
DEFENSE_INFRA:  Use the new defensive infrastructure.  Otherwise, use
                 sector efficiency for defensive values.
GODNEWS:        The world is told via news when deities are giving or taking
                 things from players.
GUINEA_PIGS:	Experimental stuff not ready for prime time.  Enable at your
		 own risk!
GO_RENEW:       Gold and Oil are renewable resources.
MOB_ACCESS:     Allows real-time updating of mobility.
MARKET:         Time-delay market and trade.
LOANS:          Allows S&L type interaction between countries.
LANDSPIES:      Creates land unit style spies.
NO_FORT_FIRE:   Forts cannot fire.
NO_LCMS:        The game doesn't need lcms to be played.
NO_HCMS:        The game doesn't need hcms to be played.
NO_OIL:         The game doesn't need oil to be played.
TECH_POP:       Technology costs more to make as your civilian population
                 grows past 50,000 civilians.
TREATIES:       Sign treaties with your friends and enemies, and breaking of
                 them is reported in the news.
.fi

.SA "Hidden, Server"
