.TH Concept Mobility
.NA Mobility "How much mobility stuff costs"
.LV Expert
.nf

Mobility represents the amount of time and energy it takes to do
something.  In "Real Life" a ground offensive takes hours.  In Empire,
it takes less than a second.  Now the way Empire *could* work would be
for you to give your troops "orders" and then wait a "realistic"
amount of time for them to carry these orders out.  But who wants to
type "nav 15 yh" and then wait 30 minutes for the ship to get there?
Certainly not you!  That's why mobility was invented.  Every update,
all of your units and sectors get more mobility, and that means that
enough time has passed for you to be able to do more stuff!

In Empire, four things have mobility: sectors, ships, planes, and land
units.  You can see how much mobility each of these gets every update
in the output of the "version" command:
                                Sectors Ships   Planes  Units
Maximum mobility                127     127     127     127
Max mob gain per update         60      120     90      90

If the mobility cost for something is a fraction, for example 2.4,
then there will be a percentage chance that the actual cost is rounded
up or down.  In our example, there would be a 60% chance that 2.4 is
rounded down to 2 and a 40% chance that it is rounded up to 3.

-- Land Units --
Land units are charged mobility for the following actions:

1. Fighting
See "info Attacking" for the mobility cost to attack.

2. Fighting rebels
For each 2 points of efficiency a land unit loses fighting rebels at
the update, it loses one point of mobility.

3. Fortification
Fortifying a land unit ("dig in") costs one point of mobility per
point of fortification.  An engineer cuts the mobility cost by one
third.  See "info fortify".

4. Converting civilians
Security units are charged 10 mobility every time civilians are converted
in a sector they are in.

5. Laying mines
A land unit is charged one point of mobility for every land mine laid.

6. Work
The increase in efficiency depends on mobility used. It costs a 100%
efficient land unit 6 mobility to raise a sector 1% efficiency.  Less
efficient land units are charged proportionally higher amounts of
mobility.  For example, a 50% land unit would be charged 12 mobility
to raise the efficiency of a sector by 1%.

7. Supply
When supply is drawn from a supply unit, that unit pays the mobility
cost to move the goods to the destination sector.  That cost is the
same as it would cost the sector to move the goods using the "move"
command.

8. Marching, reacting or retreating
The mobility cost for a land unit to march, react or retreat is:
  (path cost) * 5 * 480 / (unit speed)
where
  unit speed = speed * (1 + (tech factor))
  tech factor = (50 + tech) / (200 + tech)

See "Path Cost" below for the path cost formula.  Note that reacting
land units only pay mobility to move "to" the attacked sector; they
return to their start sector for free.  Land units on "reserve"
mission only pay half the mobility cost to react.

Marching through newly taken sectors (not old-owned, no mobility)
costs extra mobility equivalent to a path cost of 0.2.

Note that the efficiency of a land unit does not affect the mobility
costs it pays to march, unless the unit is a supply unit.  Then the
mobility costs are proportional to the efficiency of the unit.

9. Shell damage
When a land unit is shelled, its mobility goes down in exactly the
same way that its efficiency goes down (see "info Damage").


-- Planes --
Plane mobility can never go below -32.  Planes are charged mobility
for the following:

1. Air combat
For every two points of damage the plane takes from air combat, the
plane loses one point of mobility.

2. Launching satellites
A satellite is charged one point of mobility for each sector it passes
over in its launch trajectory.

3. Flying
The mobility cost for a plane to fly is:
  5 + (flight cost) * (distance flown) / (max distance plane can fly)

For escorts and interceptors, the flight cost is:
  10 / (plane efficiency)
For all other flights, the flight cost is:
  20 / (plane efficiency)

4. Land mine, pin-bombing, and nuclear damage
A land mine, pinpoint bomb or nuclear detonation
will damage the mobility of the plane in exactly the same way that the
efficiency of the plane is damaged (see "info Damage").


-- Ships --
Ships are charged mobility for the following:

1. Firing a shell
The mobility cost to fire a shell is 15, unless the NOMOBCOST option
is enabled in which case ships are charged nothing.

2. Firing a torpedo
The cost for a ship to fire a torpedo is one half of the cost for it
to move one sector.

3. Boarding a ship from a ship
See "info Attacking" for the mobility cost to board a ship from a ship.

4. Navigating and retreating
The mobility cost for a ship to navigate or retreat is:
  (sectors traveled) * 480 / (ship speed)
where
  ship speed = (base speed) * (1 + (tech factor))
  base speed = max(0.01, efficiency * speed)
  tech factor = (50 + tech) / (200 + tech)

5. Shell damage
When a ship is shelled, its mobility goes down in exactly the same way
that its efficiency goes down (see "info Damage").


-- Sectors --
Sectors are charged mobility for the following actions:

1. Converting civilians
Each civilian converted costs 0.2 mobility.

2. Shooting civilians & uws
The mobility cost to shoot civilians or uws is
  (number shot + 4) / 5

3. Enlisting military
The mobility cost for enlisting military depends on what fraction of the
civilians present you are enlisting:
  (new mobility) = (old mobility) * (1 - newmil / civs)

4. Fighting
See "info Attacking" for the mobility cost of attacking.

5. Moving commodities
Sectors move commodities in the "move" and "explore" commands, when
"supply" is drawn from them, and when they "deliver" and "distribute"
at the update.

Mobility is used from the source sector when moving.  Mobility used is
a factor of the commodity involved, the amount of the commodity, the
types of the sector and dist sector, the efficiency of both, and the
cost of the path to the destination sector.  When the move is
finished, move reports the total mobility used, and the amount of
mobility left in the sector.

The exact formula is:
  mob cost = (amount) * (weight) * (path cost) / (source packing bonus)

Weight
Each commodity has a weight:
  bar        50
  gun        10
  rad         8
  dust        5
  uw          2
  other       1

Packing Bonus
Some sectors are better at moving certain commodities than
others. This ability is known as their packing bonus. These sectors
only get these bonuses for moving items "from" them--not for moving
items "to" them. If the sector is less than 60% efficient, it doesn't
get its packing bonus.  Here are the packing bonuses:
        warehouse/harbor   bank   other
  mil          1             1       1
   uw          2             1       1
  civ         10            10      10
  bar          5             4       1
other         10             1       1

Path Cost
The path cost is the cumulative mobility cost of moving through each
sector in the path. The cost to enter a sector is based on the
sector's type and efficiency.

Sectors typically cost 0.4 at 0% efficiency, and 0.2 at 100%.
Efficient highways cost less, and mountains cost much more.  See "show
sect stats" for the exact numbers.

Road infrastructure, if enabled, allows you to construct roads in
sectors and thus decrease their mobility cost by up to 90%.

Railway infrastructure, if enabled, decreases mobility cost by up to
99%, but only for marching trains.

See "info improve" for more information on improving the
infrastructure efficiency ratings of your sectors, and see "info
sinfrastructure" for information on showing the infrastructure of your
sectors.

See "info Infrastructure" for more general information on Infrastructure.

In any case, the sector mobility cost is at least 0.001, except for
marching land units, where it is at least 0.02.

Update mobility bonus
Commodities get moved at the update through delivery and distribution.
All goods moved this way get their mob cost divided by 4 for deliver,
and 10 for distribute.  Furthermore, commodities distributed into a
harbor or warehouse get the warehouse packing bonus (normally, packing
bonuses only apply for the type of the source sector and not the type
of the destination sector).  These two bonuses represent the fact that
moving goods through an established distribution network is more
efficient.

6. Transporting planes or nukes
The weight of a plane is lcm + 2*hcm.  The weight of a nuke is the
number of "lbs" it has (from "show nuke stats").

7. Shell damage
When a sector is shelled, its mobility goes down in exactly the same
way that efficiency goes down (see "info Damage").

.fi
.SA "Sectors, Infrastructure, Ships, LandUnits, Planes, Nukes, Transportation"
