.TH Command PATH
.NA path "Show the distribution path of a sector"
.LV Expert
.SY "path <SECT>"
The path command generates a map
of the distribution path for a specific sector.
.s1
The <SECT> argument is a single sector, as described in \*Qinfo syntax\*U.

The distribution paths are indicated with \*Qgo-to\*Us
exactly as for the "route" command:
.NF
.sp
 m/     up-right        (u in level)
 m>     right           (j in level)
 m\e     down-right      (n in level)
/m      down-left       (b in level)
<m      left            (g in level)
\em      up-left         (y in level)
.FI
.sp
An example:
.sp
.EX path -2,0
.NF
.eo
     - - - 0 0 0 0 0 0 0 0 0 0 1 1
     3 2 1 0 1 2 3 4 5 6 7 8 9 0 1
 -5  .   .   a   a   .   .   .   a  -5
 -4    .   k   o   .   .   .   1    -4
 -3  a   h   j   a   .   .   +/  .  -3
 -2    ^   a   a   a   .   a/  a    -2
 -1  .   a   a   a   b   +/  +   .  -1
  0    0\  c   u>  t>  i/  .   .    0
  1  .   a>  l/  ^   a   a   .   .  1
  2    .   a   a   a   .   a   .    2
     - - - 0 0 0 0 0 0 0 0 0 0 1 1
     3 2 1 0 1 2 3 4 5 6 7 8 9 0 1
.ec
.FI
.sp
where -2,0 distributes to 10,-4 along the path shown.
.sp
.SA "distribute, census, Syntax, route, map, Distribution"
