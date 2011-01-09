.TH Command FIRE
.NA fire "Fire artillery from sector/ship/unit on sector/ship"
.LV Basic
.SY "fire [sect | ship | land] [<ASECT> | <ASHIP> | <AUNIT>] [<VSECT> | <VSHIP>]
When the multi-fire option is enabled, the syntax of the fire command
changes. It is still used to shoot up land sectors or ships. However,
you may now use multiple ships, units, sectors to fire from. Return
fire is spread amongst all the firing things.
.s1
The general form for shooting at land from a sector is:
.EX fire sect <ASECTS> <VSECT>
Where <VSECT> is the victim sector,
(the sector AT which you are shooting,
not the sector FROM which you are shooting).
ASECTS is a sector or sectors.
.s1
The general form for shooting at land from ships is:
.EX fire ship <ASHIPS> <VSECT>
.s1
The general form for shooting at land from units is:
.EX fire land <AUNITS> <VSECT>
.s1
The general form for firing at ships from land is:
.EX fire sect <ASECTS> <VSHIP>
Again, <VSHIP> is the victim ship number.
.s1
The general form for firing at ships from ships is:
.EX fire ship <ASHIPS> <VSHIP>
.s1
The general form for firing at ships from units is:
.EX fire land <AUNITS> <VSHIP>
.s1
If not specified on the command line
the program will ask for the sector(s), land unit(s),
or ship(s) FROM which you are firing, (<ASECTS>, <AUNITS>, or <ASHIPS>).
It will also prompt you for targets for each sector/unit/ship firing, if you
did not specify the target on the command line.
.s1
The only sector that may fire is a fortress,
which must be at least 5% efficient, have at least five military for a
firing crew,
at least one shell to fire,
and a gun big enough to reach the victim location;
range is equal to (7 * ((tech+50)/(tech+200)). If the
fortress is at least 60% efficient, it gets a +1 range bonus.
A fortress firing uses 1 shell.
.s1
Note that forts cannot fire if the NO_FORT_FIRE option is enabled.
See info \*QOptions\*U for more info.
.s1
Any military ship may fire as long as it has
at least one crew (which must be military, not civilians),
at least one gun,
at least one shell,
and has at least 60% efficiency.
A ship's gun range is its firing range,  as  shown  by
the show command, divided by two, times ((tech+50)/(tech+200).
A ship uses one shell per two guns firing.
.s1
Any land unit may fire as long as it has
at least one military,
at least one shell,
and has at least 40% efficiency.
A land unit's gun range is its firing range, as shown by the show
command, divided by two, times ((50+tech)/(200+tech)).  A land unit
uses the amount of ammo, as shown by the lstat command, each time
it fires.
A unit with less shells than its ammo requirement will do
proportionately
less damage.
(See \*Qshow\*U for firing ranges)
.s1
Forts, land units, and non-depth-charge-equipped ships
 cannot hit submarines on the high seas.
The only way to fire on a submarine is to have a depth-charge-equipped
ship drop a depth charge on it, costing two shells.  (Note you can
also hit subs using ASW planes or subs which have sub-torp capability.)
.s1
The program then prints a satisfying \*QKaboom!!!\*U and,
assuming you were in range, inflicts damage.
.s1
Ships lose mobility points equal to 1/2 the cost they would
pay to move 1 sector for each shot fired. (If the NOMOBCOST
option is in use, ships pay no mobility for firing.)
.s1
The damages inflicted vary with efficiency of the attacker,
the size of the guns
(battleship guns are four times as big as patrol boat guns),
the number of guns fired (7 being the useful max for forts),
and the armor of the victim.
Land sectors take damage depending on their defensive strength.
Use the command 'show sector stats' to show defensive strengths.
The higher the defensive strength, the less damage the sector
will take.
.s1
An illustrative example of shelling land:
.EX fir sect 36,18 34,18
.NF
Kaboom!!!
13% damage done to sector 34,18
.FI
Meanwhile, the owner of sector 34,18 might be logged on and would
perhaps see the following:
.NF
[15:98] Command: cen 34,18
Sat Jul 30 16:18:01 1984
CENSUS              del cnt
  sect      eff mob cmf cmf % * civ mil food min gmin fert oil
 34,18  i  100% 127 ... ...     200 100   23  23   31   73  18
	1 sector
You have a telegram waiting ...
.FI
.NF
[15:98] Command: read
BULLETIN!       dated Sat Jul 30 16:18:02 1984
Country #9 shelled 34,18
Shall I burn it? y
.FI
.NF
[15:98] Command: cen 34,18
Sat Jul 30 16:18:26 1984
CENSUS              del cnt
  sect      eff mob cmf cmf % * civ mil food min gmin fert oil
 34,18  i   87% 110 ... ...     175  83   20  23   31   73  18
	1 sector
.FI
Note that the shell did about 13% damage in the sector.

The player could have typed:
.EX fir sect #0 ?des=f 34,18

In that case, ALL forts in realm 0 would have fired on sector 34,18.
.s1
If the sector fired on is a fort with guns and shells it will fire back
with approximately the same damages (assuming it has the range).
.s1
Also, any friendly forts, units, or ships within range of the attacker
will return fire, including those belonging to allies, if the allies
are at war with the attacker as well.
.s1
If a ship fired on has guns and shells it will fire back AND
any other ships, units, or forts of the same nationality that are in range
will also fire on you.
.s1
All return fire is split evenly over all attacking sectors/units/ships, and is
adjust by the ratio of defending things to attacking things.
.s1
As a change from the original fire command, with multi-fire enabled,
you will not return fire on yourself.
.s1
IMPORTANT NOTE: Gun ranges, both on offense AND defense, are somewhat random.
If you have a range of 4.16, you can always shoot to 4, and have a 16% chance
of being able to fire at range 5...
.s1
.SA "attack, assault, torpedo, Technology, relations, Damage, Combat, Ships"
