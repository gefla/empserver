.TH Concept "Land Units"
.NA Land-units "How to use land units"
.LV Basic
.s1
Land units are organized collections of mil (militia). They are considerably
more effective and cost-efficient than militia, and have many extra
abilities. With the addition of land units, mil are now more or less
considered to be militia instead of military, and are less important.
.s1
The way that military are incorporated into units are that the mil are
loadable commodities of the unit.  The offensive or defensive
.s1
Commands for land units are generally similar to those of planes/ships, and
will usually start with 'l'.
.s1
.L "Land unit characteristics"
.s1
Land units have some characteristics of both ships and planes, plus some
unique ones. Like both ships & planes, they have efficiency & mobility.
Like ships, they can carry cargo, and may be able to fire
guns at distant targets. Like planes, they have attack and defense values,
and may be able to act automatically to defend your country.
Let's look at a typical land unit:
.s1
.EX land *
.NF
   # unit type          x,y   a   eff mil frt  mu  fd tch retr xl ln carry
   0 inf   infantry     8,0       96% 100   0  93   1  50  42%  0  0
1 unit
.FI
.s1
This land unit is an infantry unit, located at 8,0. In many games, you will
start with 1-2 basic land units of this type. Some of the things about it are
familiar. It is 96% efficient, has 93 mobility units, carries 1 food,
and is tech 50 and currently contains 100 mi. It carries 0 extra-light
planes and land units, and is not on a ship. The 'a'
stands for army, and is just like fleets or wings, i.e. a way of grouping your
units. (See info \*Qarmy\*U for more information)
Frt is the level of fortification of the land
unit. The higher the fortification, the harder the unit is to hurt.
.s1
The 'retr' stands for retreat percentage. This
land unit must begin rolling morale checks in combat whenever its efficiency
goes below 75%. This is user settable in a range determined by the happiness
of the owning country. The happier your people are, the more determinedly
they fight. If, on the other hand, you \*Qwant\*U the unit to retreat easily,
you can set this to 100% or some other high number. (For information on
setting this, see info \*Qmorale\*U. For information on morale checks, see info
\*QAttacking\*U)
.s1
.L Cargo
.s1
Each land unit can carry cargo. The cargo display for land units is very
similar to that of ships, and is gotten with the \*Qlcargo\*U command.
.s1
.EX lcargo *
.NF
lnd# unit type     x,y    a   eff  sh gun pet irn dst bar oil lcm hcm rad
   0 infantry      8,0        96%   0   0   0   0   0   0   0   0   0   0
1 unit
.FI
.s1
The land unit is not carrying anything. Potentially, land units can carry any
of the commodities listed above. Unlike ships, they cannot carry mil, civs,
or uw's. Land units are loaded and unloaded using the lload command, which
works pretty much like the load command for ships. (See info \*Qlload\*U)
Units on ships can transfer supplies to/from the ships with the \*Qltend\*U
command.
.s1
.L "HQ's & Building land units"
.s1
Like planes and ships, land units are built in special sectors. For land units,
the sector is the headquarters sectors, designation !. Like other units,
they may require hcms and lcms, and will generally require money.
.s1
Some land units may require guns to build. All this info can be gotten with the
\*Qshow\*U command.
.s1
.EX show land build
.s1
.NF
                          lcm hcm guns avail tech  $
inf   infantry             10   5    0    40    0  $500
.FI
.s1
The infantry unit takes a total of 10 lcms, 5 hcms, and  0 guns to build.
Like ships and planes, units are built at a lower percentage, and 'grow' up
to 100%, and any unit with an efficiency of less than the build percentage
is dead. For land units, this minimum efficiency is 10%.
The build requirements for land units, like those listed above, are
for a 100% unit. Thus, the infantr1 would require 1 lcm and .5 hcms
(Fractional amounts are randomly rounded... i.e. .5 has a
50% chance of being 1 or 0. It'll all even out over time)
.s1
When 'growing', land units
require materials which must be present in the sector. The
amount of growth is calculated in the same fashion as planes/ships. Generally
a land unit can grow by up to 2x the ETUs per update. (See the \*Qversion\*U
command for exact maximums, as these are deity-settable) So, for example, in
a 32 ETU game, a land unit could possibly grow by 64% per update. For the
infantry unit shown above, this would require 6.4 lcms and 3.2 hcms.
If any of these things aren't present in the sector, the unit won't
gain efficiency.
.s1
.L "Repair of Land units"
.s1
Ships can repair themselves in any sector, and can use work from their crew,
or from a harbor they are in. Planes can only be repaired in airports, and
only use work from the airport. Land units can be repaired in HQ's or
in fortresses, and use the work of the HQ or fort. This means that front-line
units in forts will repair themselves each update, assuming that the
necessary materials and work are available. They can also gain efficiency
in other sector types, but at a much reduced efficiency (1/3rd normal gain)
.s1
.L "Land unit statistics"
.s1
Each land unit has certain vital statistics which show how it will
operate. These are obtainable from the show command:
.s1
.EX show land statistics
.s1
.NF
                                       s  v  s  r  f  a  d  a  a  x
                                       p  i  p  a  r  c  a  m  a  p
                          att def vul  d  s  y  d  g  c  m  m  f  l
infantry                  1.0 1.5  60 23 15  2  1  0  0  0  0  0  0
.FI
.s1
The infantry unit has an attack multiplier of 1.0, and defensive multiplier
of 1.5. It has a vulnerability rating of 60, a speed of 23, a visibility of
15, spy value of 2, reaction radius maximum of 1, no firing range, no accuracy,
no firing damage, uses no ammo, has no aa fire,
and can't carry extra-light planes.
.s1
When attacking, a unit's value is expressed in 'mil-equivalents', i.e. 1
mil-eq is as strong as 1 mil. To find a unit's attack value, find the number of
mil currently in it, and multiply by the attack multiplier and efficiency.
In the case of our infantry unit the unit has an attack value of 96
(100*1.0*.96), and a defense value of 144
(100*1.5.96). Some units, like tanks, will have high attack multipliers,
and medium
defense multipliers. Some, like most infantry, will be better at defending than
attacking. Some, like artillery units, will be bad at both.
.s1
As time goes on, and your tech increases, the stats of your land units
will get better.  This reflects the technological breakthroughs you have
made in that you can now build better units of the same type.
.s1
The vulnerability of a unit determines how easy it is to hurt the unit by
shelling it, bombing it, or when it encounters land mines. The lower the
vulnerability, the better. Values range from 1-100.
.s1
Spd is the relative speed of the land unit. Land units move by marching (see
info \*Qmarch\*U), and use mob depending on their speed and the terrain they
are moving through. For more info and formulas, see info \*Qmarch\*U.
.s1
Vis is the visibility of the unit. This is a measure of how easy it is to
see/detect the unit. Lower is harder to see, and the numbers can range from
1-100 or so. When a land unit tries to use the \*Qllookout\*U command to look for
enemy land units/planes, its chances of success are affected by the target
units' visibility (see info \*Qllookout\*U for more details).
.s1
Spy is the other side of the equation, i.e. how well can the unit find things?
A unit with a low spy won't be very good at locating other units, and won't
be able to report much when it encounters them in battle. A high-spy unit
will be able to use the \*Qllookout\*U command with more chance of success, and
will be able to more accurately report information about the units it runs
into in battle. (See info \*QAttacking\*U for information about intelligence
reports) Units with radar capacity also use their spy value to determine how
far they can see. Spy values range from 0-127, with low numbers being most
common.
.s1
Don't confuse this spy value with the "spy" capability of a unit.  See
"info \*QSpies\*U" for more information.
.s1
Rad is the maximum reaction radius for the unit.
.s1
Frg is the unit's firing range. The unit can fire as far as the frg, modified
by the unit's tech. Accuracy is the unit's firing accuracy, and helps to
determine how much damage it will do, along with the dam of the unit. When
firing at sectors, damage is based solely on the damage. When firing in
defense of a sector, or firing at ships, damage is based on both acc and dam.
(See info \*Qfire\*U for more information)
.s1
Ammo is the number of shells a land unit uses every time it fires or
participates in combat. If the unit does not have enough shells when
firing, damage will be reduced proportionately. If a unit does not have
enough shells in combat, it will be out of supply, and have its combat
strength cut in half. (See info \*Qsupply\*U and info \*QAttacking\*U for
information on supply and how it affects combat)
.s1
Aaf is the unit's anti-aircraft fire value. The higher the value, the more
damage the unit will do to planes flying overhead.
.s1
Xpl is the number of extra-light planes the unit can carry. Planes can
operate from a land unit in the same fashion as if it were a carrier.
Normally, extra-light planes are SAMs, so units can carry SAMs for
air-defense. This particular unit can't carry any.
.s1
.L "Land unit capabilities"
.s1
There are many different capabilities that land units may have. They may
be seen with the show command:
.s1
.EX show land capability
.s1
.NF
                          capabilities
infantry                  5f light assault
.FI
.s1
The command shows the cargos the unit can carry (5 food in this case), and its
abilities. See info \*QUnit-types\*U for a complete description of the
various capabilities.
.s1
.L Moving
.s1
Land units move with the \*Qmarch\*U command, which is pretty identical to the
navigate command. Marching units can run into land-mines, be interdicted, etc.
See info \*Qmarch\*U for more information. See info \*Qmission\*U for information on
interdiction.
.s1
.L "Units & ships"
.s1
Each type of ship is rated on the number of land units it can carry.
Land units that are 'light' can be loaded onto ships using the normal \*Qload\*U
command (see info \*Qload\*U). Units that also have the 'assault' ability (see info
\*Qshow\*U and info \*QUnit-types\*U for explanations of abilities) can be
used in assaulting sectors. (See info \*Qassault\*U)
.s1
.L Supply
.s1
Units need supplies in order to attack. Each update, and each time they
attack, units will attempt to draw supplies. See info \*Qsupply\*U.
Really, I'm serious. If you don't read info \*Qsupply\*U, you'll die a horrible
death. I'm not kidding.
.s1
.L Fortification
.s1
Land units are able to \*Qfortify\*U themselves to better resist damage. Each
point of mobility spent increases the fortification level by 1, up to a maximum
equal to the maximum mobility of a land unit. A fully fortified unit takes one
half the normal amount of damage. Fortification is lost when the unit moves or
retreats. (Note that a reacting defending unit \*Qcan\*U have a fortification
value. This is an abstraction) Read info \*Qfortify\*U for more details.
.s1
.L Looking
.s1
Land units can use \*Qllookout\*U to look around. This command is analogous to the
look command used by ships. When llooking, land units have a chance to spot
other land units and also planes. Recon units tend to have better spy values,
and so are better at this.
(See info \*Qshow\*U and info \*QUnit-types\*U to
find out more about recon units)
.s1
.L Radar
.s1
Some units can use the \*Qlradar\*U command. This command works like the
radar command. Radar range is determined by the unit's spy value.
(See info \*Qshow\*U and info \*QUnit-types\*U to
find out more about radar units)
.s1
.L Engineers
.s1
Some land units have the 'engineering' ability. These units can do several
things. Engineers are the only units that can lay land-mines, and the only units
that can sweep them when moving (like a minesweeper). Attacking engineers
also halve the defender's mine bonus. Engineers can also use the \*Qwork\*U
command, which allows them to raise sector efficiency. (See info \*Qwork\*U)
They also tend to be quite expensive.
.s1
.L "Attacking & defending"
.s1
Info \*QAttacking\*U has a complete treatment of this, but I will summarize here.
When you decide to attack a sector, you can use mil and units from adjacent
sectors. The attack value of your forces is the combined attack values of all
attacking land units, plus the number of mil coming in from adjacent sectors.
.s1
The defense value is equal to the mil in the defending sector, plus
the defense values of any defending units in the sector, as well as any that
react and move there.
.s1
Friendly ships/forts/artillery units/planes can contribute
\*Qsupport\*U. Support starts at 1.0. Each friendly ship/fort/artillery unit/
plane that can help adds its damage/100 to the support. For example, if you
were attacking a sector, and a friendly battleship was close enough to fire
there, it could support you. If it would normally do 23% damage when firing,
it would add +.23 support, making your support number 1.23. The enemy also
gets support from all his stuff.
.s1
When all support is totaled, the attacker strength is multiplied by
the attacker's support to get the total attacker strength. The defender's
total strength is determined similarly. Both sides have their strength modified
by the terrain. (show sector stats will show the defensive value of a sector)
He can also get support
by having mine-fields in the sector (see info \*Qlmine\*U and info
\*QAttacking\*U for more information). Planes support only if assigned to a
support mission. See info \*Qmission\*U for information on support missions.
.s1
Combat is done fairly normally, with a series of rolls for attacker/
defender casualties, until one side is dead or retreats. Land units take damage
after all mil on their side are dead, and each 'hit' on a land unit does 1 mil
damage. (Therefore, a unit that takes 100 mil to build would take 1% damage
when it suffers a casualty. A unit that takes 50 mil would take 2%, etc)
When a land unit's level of damage taken in a battle reaches it's 'retreat
level', it must begin taking morale checks. When it fails one, it will retreat.
Retreating attacking units go back to the sector they came from. Defending
units will attempt to retreat to an adjacent owned sector. If there are none,
the unit will take extra damage and continue fighting.
.s1
If the attacker wins, he captures the sector, and his mil/units will
move into the sector if he ordered that. If the defender wins a combat,
his units in the sector stay there. His reacting defending units that did not
fail a morale check return to their starting point. Defending units who
started in the sector stay in the sector.
.s1
In both cases, retreated units end up in the sector they retreated to.
.s1
.L "Units and retreating"
.s1
Ships can use the \*Qretreat\*U command to specify when and how they will
run away from trouble. Land units use a similar command \*Qlretreat\*U to
do the same thing. See info \*Qlretreat\*U for more details.
.s1
.SA "Unit-types, show, supply, LandUnits"
