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
power factor = (  power value of money
                + power value of sectors
                + power value of commodities in sectors
                + power value of commodities loaded on ships and land units)
               * (20 + nation tech level) / 500
             + power value of ships, planes and land units
               * (20 + their tech level) / 500
.FI
.s1
The power value of money is dollars / 100.
.s1
The power value of a sector is efficiency / 10.
.s1
The power value of a ship or land unit is (lcm cost / 10 + hcm cost +
10) * efficiency / 100 * 2.
.s1
The power value of a plane is 20 * efficiency / 100 * (20 + nation
tech level) / 500.
.s1
The power value of commodities is civilians / 10 + military / 10 +
shells / 12.5 + guns / 2.5 + petrol / 500 + iron / 100 + dust / 5 +
bars + oil / 10 lcms / 10 + hcms / 5.
.s1
Efficiency is in percent.
.s1
.SA "census, nation, Communication, Nations, Diplomacy"
