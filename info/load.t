.TH Command LOAD
.NA load "Load goods, people, etc onto a ship"
.LV Basic
.SY "load <COMMODITY> <SHIPS> <AMOUNT>"
.SY "load plane <SHIPS> <PLANES>"
.SY "load land <SHIPS> <UNITS>"
The load command carries out the loading of ships.
Commodities (like guns, civs, food) can be loaded onto any ship.
Missiles can be loaded onto nuke subs
and planes can be loaded onto carriers.
Land units may be loaded onto many ships.
.s1
The ship(s) must be in a harbor and
the harbor must be at least 2% efficient.
.s1
.SY "load <COMMODITY> <SHIPS> <AMOUNT>"
In the first form,
the amount argument specifies the amount of the given commodity
you wish to load on each ship specified.
Amounts greater than that allowed will be truncated to the
maximum possible. Amounts less than 0 indicate that you want to
make the ships have that amount. If the ships have less
than that amount, more will be loaded. If the ships have more
than that amount, the commodity will be unloaded. For example,
.s1
.EX load food 3 -100
.s1
will cause ship 3 to load or unload food so that it has 100.
If it had 120 food, 20 would be unloaded. If it had 50 food,
50 would be loaded.
.s1
.SY "load plane <SHIPS> <PLANES>"
In the second form,
where you say \*Qplane\*U instead of say \*Qciv\*U planes are
loaded onto ships. Multiple planes may be given. Some
may not be loaded, if there is not enough room.
.s1
.SY "load land <SHIPS> <UNITS>"
In the third form,
where you say \*Qland\*U instead of say \*Qciv\*U, a unit
is loaded onto a ship. Multiple units may be given. Some may
not be loaded, if there is not enough room. (Note that planes on
land units on ships can't be used, and units on ships generally can't
do much, like firing, marching, etc).  When loading a land unit onto a
ship, the unit will check to see that it is in supply before stepping
on deck.  If it isn't, then it will try to resupply itself (see info
supply) and if it is not able to do that, then you are issued a
warning.  Sailors don't like to go to sea without supplies.
.s1
See \*Qinfo Ship-types\*U for a table of the maximum amounts of
each items that can be carried on each ship type.
.s1
If you are loading things onto FRIENDLY ships, you need to type the
numbers of the ships, fleets or realms
will not work. I.e. if you do "load f s 999", it'll load 999 food onto
all your ships in fleet s. If you do "load 1/2/3/4 999 f", it'll try to
load food onto ships 1, 2, 3, and 4, regardless of whether they're yours
or they belong to a friendly trading partner.
.s1
If you load something onto a friendly nation's ship, it becomes
his and he gets a telegram. If you load a unit, he gets any planes it
may be carrying.
.s1
If you unload a plane or unit in a friendly nation's harbor, it
becomes his. If you unload a unit, he gets any planes it may be carrying.
.s1
Note that if any of <SHIPS>, <UNITS> or <PLANES> does not start with a
number (e.g. it is of the form "s" or "*" or "#1" and not of the form
123 or 12,14) then you will not be given any error messages.  This is
so that you can type something like "load land * *" without getting a
hundred error messages.
.s1
.SA "unload, navigate, Ship-types, Unit-types, Plane-types, Ships, Moving"
