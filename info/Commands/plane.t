.TH Command PLANE
.NA plane "Report status of plane, wing or planes in a give area"
.LV Basic
.SY "plane [<PLANE> | <SECTS>]"
The plane report command is a census of your planes
and lists all the information available in readable format.
.s1
The <PLANE> and <SECTS> arguments are provided in case you only
wish to look at one plane
or all planes within a given area.
.s1
Plane expects some argument.  To see all planes, use the
argument '*', or enter "plane *".
.EX plane *
.NF
   #    type                x,y    w  eff  mu def tech ran hard   s/l LSB nuke
   0 f2  P-51 Mustang       1,-1     100%  90   5  110  11    0
   1 f2  P-51 Mustang       1,-1     100%  90   5  110  11    0
   3 lb  TBD-1 Devastato    1,-1     100%  90   4  120  11    0
3 planes
.FI
.s1
The report format contains the following fields:
.s1
.in +1i
.L #
the plane number
.L type
the type of plane; \*Qfighter 1\*U, \*Qjet hvy bomber\*U, etc,
.L x,y
the plane's current location (relative to your capital),
.L w
the \*Qair wing\*U designation letter,
.L eff
the plane's efficiency,
.L mu
the number of mobility points the plane has,
.L att
the attack value of the plane (for air-to-air combat)
.L tech
the tech level at which it was created,
.L range
and the range (in sectors) it can fly on a given mission.
.L s/l
the ship or land unit the plane is on
.L LSB
For satellites, LS stands for "launched?" and "Geo-synchroneous orbit?".
For planes or missiles armed with nukes, B will either have the value 
"A" for airburst, or "G" for groundburst (see info arm).
.L nuke
the type of nuke carried
.in
.s1
.SA "pstat, upgrade, arm, bomb, build, drop, fly, launch, paradrop, recon, satellite, scrap, wingadd, Planes"
