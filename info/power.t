.TH Command POWER
.NA power "Display arbitrarily measured strengths of countries"
.LV Basic
.SY "power [new|update] [<NUM>|country <NATS>]"
The power report provides one view of national strengths.
It can be particularly helpful in planning defense strategies.
.s1
Normally, the last saved power report is shown.
The optional arguments \*Qnew\*U and \*Qupdate\*U request a new power
report based on up-to-date information.
This costs 10 BTUs.
The new report is saved for use by future power commands, except when
a deity uses \*Qupdate\*U.  This lets deities examine up-to-date power
reports without affecting what players get to see.
.s1
If option AUTO_POWER is enabled, the command doesn't let you save new
power reports.  Instead, the power report is updated automatically
right before the update.
.s1
If the optional \*QNUM\*U argument is given, only the
top NUM entries in the power chart will be displayed.  Note that for
deities, if you give a negative number, only the top NUM entries in
the power chart will be displayed, and the power number rating will
not be displayed.
.s1
If the optional \*Qcountry\*U argument is given, then the information
is given for those particular countries.
.s1
In the power report the countries are listed
in order of decreasing \*Qpower factor\*U.
Powerless countries are not shown unless explicitly requested with the
\*Qcountry\*U argument.
.s1
The power report shows the following columns:
.s1
.in \w'money\0\0'u
.L sects
number of sectors
.L eff
average sector efficiency
.L civ
number of civilians
.L mil
number of military
.L shell
number of shells
.L gun
number of guns
.L pet
amount of petrol
.L iron
amount of iron
.L dust
amount of dust
.L oil
amount of oil
.L pln
number of planes
.L ship
number of ships
.L unit
number of land units
.L money
cash on hand
.in
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
.SA "census, nation, Communication, Nations, Diplomacy"
