.TH Command POWER
.NA power "Display arbitrarily measured strengths of all countries"
.LV Basic
.SY "power [num]"
.SY "power new [num]"
.SY "power country <NATS>"
The power report provides one view of national strengths.
It can be particularly helpful in planning defense strategies
and treaty voting.
.s1
If the optional \*Qnum\*U argument is given, only the
top num entries in the power chart will be displayed.  Note that for
deities, if you give a negative number, only the top num entries in
the power chart will be displayed, and the power number rating will
not be displayed.
.s1
If the optional \*Qnew\*U argument is given,
the program will take several minutes to gather information
and generate a power report based on that up-to-date information.
This costs several BTU's (typically 10).
.s1
If the optional \*Qnew\*U argument is not given,
the program will shown you the report generated
the last time someone asked for new data.
.s1
If the optional \*Qcountry\*U argument is given, then the information
is only given for those particular countries.  Countries are
separated by the "/" character as in:
.EX power c Fodder/19/Mirky
Note that countries may be specified as either names or numbers.
.s1
In the power report the countries are listed
in order of decreasing \*Qpower factor\*U,
each being represented by a line in the following format:
.s1
.NF
sects  eff civ  mil  shell gun pet  iron dust oil  pln ship unit money
.FI

Each column represents a different commodity or aspect for that country.
The columns and values used in computing the \*Qpower factor\*U are:
.NF
a	# of sectors
b	average sector efficiency
c	# of civilians
d	# of military
e	# of shells
f	# of guns
h	tons of iron ore (not displayed)
i	tons of gold dust
j	# of gold bars
k	tons of food (not displayed)
l	tanks of oil
g	# of planes
m	# of ships
mm	tons of ships (not displayed)
r	# of land units
n	# of dollars
oo	units of light construction materials (not displayed)
pp	units of heavy construction materials (not displayed)
qq	megatons of nuclear weaponry (not displayed)
bb	gold bars
.FI
.s1
The \*Qpower factor\*U is determined by the following equation:
.s1
.NF
power factor = for each land unit calculate and add the following
                ((((land unit lcm cost / 10) * (land unit effic / 100)) +
                  ((land unit hcm cost / 10) * (land unit effic / 100))) * 2)
power factor += for each ship calculate and add the following:
                ((((ship lcm cost / 10) * (ship effic / 100)) +
                  ((ship hcm cost / 10) * (ship effic / 100))) * 2)
power factor += for each plane calculate and add the following:
                (20 * (plane effic / 100) * (nation tech level / 500))
power factor += ((nation money / 100) + (petrol / 500) +
                 ((civilians + military) / 10) + (shells / 12.5) +
                 (iron / 100) + (dust / 5) + (oil / 10) + bars +
                 (guns / 2.5) + (lcms / 10) + (hcms / 5))
power factor += ((number of sectors * (average sector effic / 100)) * 10)
power factor = ((power factor) * (nation tech level / 500))
power factor = ((power factor) * (research factor))

.FI
.s1
The research factor is either 1, if RES_POP is not being used, or 1+(max 
pop in a sector)/10000, if it is being used.
.s1
.SA "census, nation, Communication, Nation, Diplomacy"
