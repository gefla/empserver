.TH Command FUEL
.NA fuel "Give fuel to a ship/unit"
.LV Basic
.SY "fuel <s|l> <SHIP/FLEET | UNIT/ARMY> <AMOUNT> [<OILER>]"
The fuel command carries out the fueling of ships and units.
The ship(s) must be in a harbor and
the harbor must be at least 2% efficient.
Land units may be in any sector.
Alternatively, you can fuel at sea using an oiler, which is
any ship with the oiler ability. (Note: if you are in a harbor,
you can't fuel from an oiler, you must use the harbor)
.s1
Ships and units can be fueled by either petrol or oil, with petrol being
used first. 1 unit of petrol = 5 fuel units. 1 unit of oil = 50
fuel units. Each ship or unit, in turn, will attempt to take on the 
specified amount of fuel. Petrol/oil will be used for each ship or unit,
the amount being rounded up individually. For example, if you have
a harbor with 50 fuel in it, and you tell a ship with a 50 fuel
capacity to load 50 fuel, it'll use 1 oil. If you tell it to load
*1* fuel 50 times, it'll use *50* oil, with the round-up making
it use 1 oil each time. The solution, of course, is to not load
fuel in tiny, wasteful amounts. 
.s1
Each update, ships and units use fuel to make mobility points. To produce
10 mobility points requires the number of units of fuel listed
for the ship or unit in the capability list. (see info show)
(This number is usually 1, but may be more). If the number is a 0,
the unit does not use fuel.
.s1
If the ship or unit has no fuel,
it will attempt to get fuel using the supply procedure (see
info supply). It will try to use first petrol, then oil, until it has enough
fuel to get full mobility.
If it can't get fuel by using the supply system, it won't get mobility.
.s1 
See \*Qshow\*U for a table of the maximum amounts of
fuel that can be carried on each type of unit or ship.
.s1
You can fuel friendly ships, but must fuel them one at a time, by number.
.s1
.SA "Ship-types, Unit-types, Ships, LandUnits"
