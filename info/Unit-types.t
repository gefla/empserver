.TH Concept "Unit Types"
.NA Unit-types "Description of the different types of land units"
.LV Basic
Some of the units currently available in Empire are:
.s1
.nf
infantry	The Bloody Infantry. Lotsa guys on foot with guns. They're
		slow, and they carry their supplies in horse-drawn wagons,
		which slows them down even more. Slow. They're also not
		terribly good at scouting. They don't attack very well,
		with a multiplier ranging from 1 to 1.5 or so with better
		tech. However, they really shine on defense, with a multiplier
		from 1.5 to 3.0. If you want to hold a line... use infantry.
		They're fairly resistant to shelling and bombing, with the
		later types of infantry units being better.

cavalry		Empire starts just at the end of the age of cavalry. They're
		quick, and they scout pretty well. They're better at attacking
		than defending. They are pretty easy to kill with shelling or
		bombing. Still, if you want a low-tech unit to go somewhere
		quick, they're the guys.

supply		Supply units range from horse and wagon to truck units. Early
		supply units are slow, but the later supply unit types are
		motorized and are faster. Supply units carry a bunch of stuff,
		but try your best not to get them in combat, because they don't
		enjoy it, and they're REALLY bad at it. Supply units can give
		supplies to land units, as well as forts/ships/sectors. See
		info supply.

lt armor	Armor units may be light, medium, or heavy. Light armor units
armor		have much the same uses as cavalry, although they are a good
hvy armor	bit less fragile. Medium armor is medium-fast, good for general
		attacking, while heavy armor is slow, but good for assaults,
		any situation where you need to destroy a heavily-defended
		fixed point.

		All armor is better at attacking than defending. Higher tech
		types of armor units are better at defending, however. Armor
		gets much of its combat advantage from its mobility, being
		able to attack and move in, severing supply lines & blocking
		reaction & reinforcements.

		Armor is fairly resistant to shelling/bombing.

artillery	Artillery is a long range killer. You can use it to shell
lt artillery	sectors, but it can often be more effective when used as
hvy artillery	support for your attacks. Lt artillery has more mobility,
mech artillery	but less damage. Hvy artillery has less mobility, but more
		damage. Mechanized artillery gives you good damage and good
		mobility, but is very expensive.

		The better the accuracy of the artillery, the better it is
		at assisting your troops or shelling ships. The better the
		damage rating, the better the damage done. High-damage, low
		accuracy artillery is mostly good for shelling sectors, while
		high-accuracy artillery is good at supporting your troops,
		or shooting at ships.

		All artillery is bad at attacking, but can usually defend in
		a not-horrible fashion. Artillery units are, however, pretty
		vulnerable to shelling and bombing. (except for mech artillery)
		
engineers	Engineers are the most versatile unit. They can lay mines,
mech engineers	clear mines, defend well, attack well, work on sectors, etc.
		The drawbacks? Well, they're VERY expensive and not very
		big. Mech engineers have vehicles and are much faster and
		more resistant to damage.

mech infantry	These two types of infantry are faster. Mech infantry is
motor infantry	also more resistant to damage and even better on defense.

marines		These units are like improved infantry, but more expensive and
		harder to make. Their best use is for assaulting beaches, but
		they do make superior infantry if you have to use them that
		way.

security	These units are good at fighting guerrillas, but not for
		much else. They fight about as well as low-tech infantry,
		and their smallish size makes them not worth much.

		They do help you when shooting helpless civs (see info shoot),
		and fight guerrillas during the update. The security troops
		get a chance to 'raid' the guerrillas and kill some of them
		before the guerrillas fight. This will reduce your casualties
		from guerrilla war.

radar units	These units are high-tech. They give you the ability to
		map the enemy easily as you advance. Try not to get them
		into combat, as they're horrible at it, and they're very costly.
		They're generally motorized, so they're pretty fast, and they
		all get shelled/bombed hard.

aa unit		These units are good defensively, and fight planes well. They
		are a mechanized unit, and move & survive pretty well. Only
		units with the FLAK ability can fire AA.

Use the "show" command to see units stats and capabilities.

.fi
.s1
These units types differ in their fundamental
capabilities; some can fire guns, some cannot;
each has it's own specific abilities & roles.
The capabilities of each unit are given by the show command.
The headings are as follows. For the building data:
.s1
.EX show land build
.s1
.in \w'avail\0\0'u
.L lcm
The lcm required to build the land unit.
.L hcm
The hcm required to build the land unit.
.L guns
The guns required to build the land unit.
.L avail
The avail (work) required to build the land unit.
.L tech
The minimum technology required to build the land unit.
.L $
The cost of the land unit.
.in
.s1
The headings for the stats listing are
.s1
.EX show land stats
.s1
.in \w'fc/fu\0\0'u
.L att
the attack multiplier of the land unit
.L def
the defensive multiplier of the land unit
.L vul
the vulnerability of the unit to bombing/shelling (1-100)
.L spd
the distance/mu for moving (in relative units)
.L vis
how visible the land unit is, (relative units)
.L spy
how far the land unit can see, (i.e. how good the
reconnaissance equipment is), again in relative units
.L rad
the unit's reaction radius
.L rng
twice the distance the guns can fire,
The range formula is (rng/2) * ((tech + 50) / (tech + 200))
.L acc
the firing accuracy of the unit
.L fir
the number of guns that fire when the unit fires
.L amm
ammo: the number of shells the unit uses when fighting or firing
.L aaf
the aa fire rating of the unit
.L fc/fu
The land unit's fuel capacity & usage.
(see info fuel) (If the FUEL option is enabled)
.L xpl
The number of extra-light planes the land unit can carry.
(If the XLIGHT option is enabled)
.L lnd
The number of land units the unit can carry (no 'heavy' units)
.in
.s1
Each land unit can carry a certain amount of products and has certain
capabilities. These are listed under the cargoes & abilities section.
The cargoes give the number of each product that can be carried.
The abilities are as follows.
.s1
.EX show land capabilities
.s1
.in \w'engineer\0\0'u
.L xlight
the unit can carry xlight planes. (If the XLIGHT option is used)
.L engineer
the unit can lay/sweep mines, work on sectors/fortifications/etc.
.L supply
the unit can supply other units
.L security
the unit is good at fighting guerrillas and shooting helpless civilians
.L light
the unit can be carried on ships that don't have supply capability
.L marine
the unit is good at assaulting
.L recon
the unit is good at gathering intelligence. It will do better using llookout,
and will report enemy units in battle better.
.L radar
the unit has radar capability
.L assault
the unit may be used in assaults
.L flak
the unit can fire 'general unit flak' (see "info Flak")
.L spy
the unit is a spy
.L train
the unit is a train, and can't be loaded on land units
.L heavy
the unit cannot be carried on land units or ships, not even supply ships
.in
.s1
Some examples of the output:
.s1
.EX show land build
.NF
                          lcm hcm mil guns shells avail tech  $
War band                    5   0 100    0      0     1    0  $500
Supply Wagon                5   5  50    0      0     2    0  $500
GPG Commandos              15  15  25    1      2     5  150  $800
artillery 1                40  60  70   20     40    17  100  $1000
Jump infantry              35  25 100    5     10     9  400  $3000
marines 1                  20  10 100    2      4     5  200  $2000
.FI
.s1
.EX show land stats
.NF
                                       s  v  s  r  f  a  d  a  a        x
                                       p  i  p  a  r  c  a  m  a  f  f  p
                          att def vul  d  s  y  d  g  c  m  m  f  c  u  l
War band                  1.3 1.0  99 23  5  1  1  0  0  0  0  0  0  0  0
Supply Wagon              0.1 0.2 100 15  6  1  0  0  0  0  0  0  0  0  0
GPG Commandos             1.0 1.0  70 30  1  2  1  0  0  0  5  1 10  1  0
artillery 1               0.0 0.5  50 15  6  1  0  6 40  4  5 15  0  0  0
Jump infantry             5.0 6.0  20 55  1  4  4  0  0  0  5 35 10  1  5
marines 1                 1.0 2.0  70 23  3  3  2  0  0  0  5  5  0  0  2
.FI
.s1
.EX show land cap
.NF
                          capabilities
War band                   5f engineer light recon
Supply Wagon               50s 5g 100f 200l 100h 100r 200d 10b 100i supply light
GPG Commandos              5f security
artillery 1                5f 10g 20s
Jump infantry              5f 2g 10s xlight
marines 1                  5f 10s xlight light marine
.FI
.s1
.SA "land, LandUnits"

