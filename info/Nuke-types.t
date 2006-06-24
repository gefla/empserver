.TH Concept "Nuke Types"
.NA Nuke-types "Description of different kinds of nuclear weapons"
.LV Expert
There are two different types of nuclear devices in Empire:
fission bombs, fusion bombs.
Both fission and fusion bombs perform similarly,
with fusion bombs giving \*Qmore bang for the buck\*U.
.s1
In general, the only way to avoid damage by a nuclear detonation
is to avoid the area prior to and during actual detonation.
\*QHardening\*U missiles (in silos) is the only exception,
and even that cannot protect from a direct hit
by the larger nuclear devices.
.s1
The details of the nuclear devices present in Empire can be seen using
the show command.
The meaning of the fields are as follows:
.s1
.in \w'name\0\0'u
.L name
The nukes name.
.L lcm
The amount of lcm needed to build the nuke
.L hcm
The amount of hcm needed to build the nuke
.L oil
The amount of crude oil required to build it.
.L rad
The number of radioactive products required to build it.
.L tech
The minimum tech level required to construct the nuke.
.L $
The cost of the nuke to construct.
.s1
.in
The other fields given in the capabilities are as follows.
.s1
.in \w'blst\0\0'u
.L blst
The blast type of the nuke
.L dam
The damage rating of the nuke
.L lbs
the weight of the warhead, limiting missile and plane nuclear
device carrying capacity.
.in
.s1
As a guide (these numbers are subject to change):
.EX show nuke build
.NF
Printing for tech level '300'
              lcm hcm  oil  rad avail tech res $
10kt  fission  50  50   25   70    49  280   0 $ 10000
15kt  fission  50  50   25   80    51  290   0 $ 15000
50kt  fission  60  60   30   90    60  300   0 $ 25000
.FI
.s1
.EX show nuke capab
.NF
Printing for tech level '300'
              blst dam lbs tech res $        abilities
10kt  fission    3  70   4  280   0 $  10000
15kt  fission    3  90   5  290   0 $  15000
50kt  fission    3 100   6  300   0 $  25000
.FI
.s1
The larger the megatonnage, the more damage done by the weapon.
Effects (as in the real world) can only be determined
by experimentation,
although research suggests that radioactivity left behind by nuclear
detonations may have lasting effects.
.s1
.SA "launch, fly, build, transport, show, Planes, Nukes"
