.TH Command PSTAT
.NA pstat "Report statistics of plane, wing or planes in a give area"
.LV Basic
.SY "pstat [<PLANE> | <SECTS>]"
The pstat command is a report of the varying statistics of your planes and
lists the info in a readable format.  Since your planes' statistics vary by
tech level, this is a very useful way to see what the attack, defense, etc.
modifiers are on a per-plane basis.
.s1
The <PLANE> and <SECTS> arguments are provided in case you only
wish to look at one plane
or all planes within a given area.
.s1
pstat expects some argument.  To see all planes, use the
argument '*', or enter "pstat *".
.EX pstat *
.NF
pln# plane-type                x,y    eff  tech att def acc ran load fuel
   0 f2  P-51 Mustang          1,-1   100%  110   5   5  72  11    1    1
   1 f2  P-51 Mustang          1,-1   100%  110   5   5  72  11    1    1
   3 lb  TBD-1 Devastator      1,-1   100%  120   0   4  45  11    2    1
3 planes
.FI
.s1
The report format contains the following fields:
.s1
.in +1i
.L pln#
the plane number
.L type
the type of plane; \*Qfighter 1\*U, \*Qjet hvy bomber\*U, etc,
.L x,y
the plane's current location (relative to your capital),
.L eff
the plane's efficiency,
.L tech
the tech level at which it was created,
.L att
the attack value of the plane (for air-to-air combat)
.L def
the defensive value of the plane (for air-to-air combat)
.L acc
the accuracy of the plane (for bombing)
.L ran
and the range (in sectors) it can fly on a given mission.
.L load
the carrying capacity of the plane
.L fuel
the amount of fuel the plane takes per flight
.in
.s1
.SA "plane, upgrade, arm, bomb, build, drop, fly, launch, paradrop, recon, satellite, scrap, wingadd, Planes"
