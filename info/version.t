.TH Command VERSION
.NA version "Display parameters of this world"
.LV Expert
.SY "version"
The version command prints out the current settings of parameters
affecting the Empire game.
Included are the creation date for this version,
the size of the world, (X by Y),
etc.
.s1
For example:
.s1
.EX version
.NF
Wolfpack Empire 4.3.32

The following parameters have been set for this game:
World size is 64 by 32.
There can be up to 99 countries.
By default, countries use their own coordinate system.

Use the 'show' command to find out the time of the next update.
The current time is Sat May 25 16:42:56.
An update consists of 60 empire time units.
Each country is allowed to be logged in 1440 minutes a day.
It takes 8.33 civilians to produce a BTU in one time unit.

A non-aggi, 100 fertility sector can grow 0.12 food per etu.
1000 civilians will harvest 1.3 food per etu.
1000 civilians will give birth to 5.0 babies per etu.
1000 uncompensated workers will give birth to 2.5 babies.
In one time unit, 1000 people eat 0.5 units of food.
1000 babies eat 6.0 units of food becoming adults.
No food is needed!

Banks pay $250.00 in interest per 1000 gold bars per etu.
1000 civilians generate $8.33, uncompensated workers $1.78 each time unit.
1000 active military cost $83.33, reserves cost $8.33.
Up to 50 avail can roll over an update.
Happiness p.e. requires 1 happy stroller per 10000 civ.
Education p.e. requires 1 class of graduates per 10000 civ.
Happiness is averaged over 48 time units.
Education is averaged over 192 time units.
The technology/research boost you get from the world is 50.00%.
Nation levels (tech etc.) decline 1% every 96 time units.
Tech buildup is limited to logarithmic growth (base 2.00) after 1.00.

				Sectors	Ships	Planes	Units
Maximum mobility		127	127	127	127
Max mob gain per update		60	90	60	60
Max eff gain per update		--	100	100	100
Maintenance cost per update	--	6.0%	6.0%	6.0%
Max interdiction range		8	8	--	8

The maximum amount of mobility used for land unit combat is 5.00.

Fire ranges are scaled by 1.00.
Flak damage is scaled by 1.75.
Torpedo damage is 2d40+38.
The attack factor for para & assault troops is 0.50.
12% of fallout leaks into each surrounding sector.
Fallout decays by 28% per update.

Damage to			Spills to
	      Sector  People  Mater.   Ships  Planes  LandU.
Sector		 --	100%	100%	  0%	  4%	 30%
People		 10%	 --	 --	 --	 --	 --
Materials	 10%	 --	 --	 --	 --	 --
Efficiency	 10%	 --	 --	 --	 --	 --
Ships		 10%	100%	100%	 --	  0%	  0%
Planes		 10%	  0%	  0%	 --	 --	 --
Land units	 10%	  0%	100%	 --	  0%	  0%

You can have at most 640 BTUs.
You are disconnected after 15 minutes of idle time.
Visitors are disconnected after 5 minutes of idle time.

Options enabled in this game:
	ALL_BLEED, BLITZ, EASY_BRIDGES, FALLOUT, GODNEWS, INTERDICT_ATT,
	NOFOOD, NOMOBCOST, NO_PLAGUE, RAILWAYS, SAIL

Options disabled in this game:
	AUTO_POWER, BRIDGETOWERS, GO_RENEW, GUINEA_PIGS, HIDDEN, LOANS,
	LOSE_CONTACT, MARKET, MOB_ACCESS, NO_FORT_FIRE, RES_POP, SUPER_BARS,
	TECH_POP

See "info Options" for a detailed list of options and descriptions.

The person to annoy if something goes wrong is:
	Wolfpack <wolfpack@wolfpackempire.com>.

You can get your own copy of the source from <http://www.wolfpackempire.com/>.

Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
              Ken Stevens, Steve McClure, Markus Armbruster
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
.FI
.s1
.SA "Innards, Basics, Playing"
