.TH Command RANGE
.NA range "Edit the range of a plane"
.LV Expert
.SY "range <PLANES> <range-limit>"
The \*Qrange\*U command is used to limit the maximum range of
individual planes. This can be useful in setting up high tech
interceptors (planes, ABMs, SAMs, etc.) to cover only a small
limited area.
.s1
Note that missiles always fly at double range (since they don't need
to fly back), so you should cut the desired range in half before using
the range command.
.s1
For example, if plane 1 were a plane and you wanted to limit its
reaction radius to 6, then you would type:
.EX "range 1 6"
and if plane 2 were a missile and you wanted to limit its reaction
radius to 6, then you would type:
.EX "range 2 3"

.SA "Planes"
