.TH Command CONVERT
.NA convert "Change conquered civilians into uncompensated workers"
.LV Basic
.SY "convert <SECTS> <NUMBER>"
The convert command converts \*Qenemy\*U (captured) populace into
uncompensated workers.
If the sector is not actually conquered, no civilians will be
converted.  If you convert all the civs in a sector, then the sector
is no longer occupied (i.e. you become the "oldowner" of the sector).
.s1
After the program has collected any necessary information
it will list each sector where civilians are converted.
.s1
Note: you must have enough military in the sectors to force the 
populace to become uncompensated workers -- about 10%. Land units
count toward this at a value equal to the number of mil required
to build them. Security units are even more efficient, counting for
more than their base value, and increase the number converted.
.s1
There must also be mobility in the sector to be converted (see \*Qinfo
Mobility\*U for the exact formula).
.s1
.s1
.EX conv -3:3,-1:2 10
.NF
Number to convert : 22
 22 conquered populace converted in -1,-1 (30)
 13 conquered populace converted in 1,-1 (13)
 22 conquered populace converted in 3,-1 (30)
 17 conquered populace converted in -2,0 (17)
 22 conquered populace converted in 0,0 (36)
 22 conquered populace converted in -1,1 (34)
 22 conquered populace converted in 1,1 (34)
 18 conquered populace converted in 0,2 (18)
 22 conquered populace converted in 2,2 (45)
Total new uw : 180
.FI
Note that there is a cost of 0.01 BTU's per civilian converted
in addition to the registration fee of $1.5 per conversion.
.SA "Populace, Mobility, Occupation"
