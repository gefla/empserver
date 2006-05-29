.TH Command PLANE
.NA plane "Report status of planes"
.LV Basic
.SY "plane <PLANES>"
The plane report command is a census of your planes and lists.  For
example:
.EX plane *
.NF
   #    type                x,y    w   eff  mu def tech ran hard carry special
   0 f2  P-51 Mustang       1,-1      100%  90   5  110  11    0
   1 f2  P-51 Mustang       1,-1      100%  90   5  110  11    0
   3 lb  TBD-1 Devastato    1,-1      100%  90   4  120  11    0
3 planes
.FI
.s1
The report format contains the following fields:
.s1
.in +1i
.L #
the plane number
.L type
the type of plane; \*Qfighter 1\*U, \*Qjet hvy bomber\*U, etc.,
.L x,y
the plane's current location,
.L w
the \*Qair wing\*U designation letter (set by \*Qwingadd\*U command),
.L eff
the plane's efficiency, prefixed by \*Q=\*U if stopped,
.L mu
the number of mobility points the plane has,
.L def
the defense value of the plane (for air-to-air combat)
.L tech
the tech level at which it was created,
.L range
and the range (in sectors) it can fly in one sortie,
.L carry
the ship or land unit the carrying the plane, if any,
.L special
shows \*Qorbit\*U or \*Qgeosync\*U for satellites in (geo-synchronous)
orbit, else nuclear armament type and programming (\*QA\*U for
air burst, \*QG\*U for ground burst; see info \*Qarm\*U).
.in
.s1
.SA "pstat, upgrade, arm, bomb, build, drop, fly, launch, paradrop, recon, satellite, scrap, wingadd, Planes"
