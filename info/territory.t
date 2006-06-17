.TH Command TERRITORY
.NA territory "Assign various sectors to a specific territory"
.LV Expert
.SY "territory <SECTS> <NUMBER> [<FIELD>]"
In Empire, sectors may be given a territory id number.
This number may be used in conditional statements as in:
.EX census # ?terr=27
The program will then give you a census report for all the
sectors that have been assigned the territory number \*Q27\*U.
.s1
In this sense it is essentially an extended \*Qrealm\*U.
However, you are not limited to rectangular regions and you have 
99 values to choose from.
.s1
There are four independent territory fields terr, terr1, terr2, terr3
selected by field arguments 0, 1, 2, 3, respectively.  Deities can use
an additional territory field dterr, selected by -1.  If the optional
field argument is empty, terr is assumed for mortals and dterr for
deities.
.s1
.EX territory #1 5 1
.s1
Would set terr1 to 5 for all sectors in realm 1.
.s1
.EX sect * ?terr1=6&terr2=12
.s1
If terr1 was used to mark different islands and terr2=12 indicated
productive sectors, this would highlight all the productive sectors
on island number 6.
.s1
Some possible uses include;
all sectors distributing to a particular sector;
all coastal sectors;
all sectors that have armed missiles in them.
And probably thousands that I haven't thought of. ;-)
.s1
To \*Qunset\*U a territory, set its value to zero.
.s1
Note: terr and terr0 are the same.
.s1
.SA "realm, distribute, Sectors"
