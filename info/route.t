.TH Command ROUTE
.NA route "Display delivery routes"
.LV Expert
.SY "route <ITEM> <SECTS>"
The route command generates a graphic display
of the delivery arrangement
for a specific deliverable item
in a given sector area.
.s1
The <SECTS> argument is as described in \*Qinfo syntax\*U.
<ITEM> is one of the following:
.NF
c    civilians
m    military
u    uncompensated workers
f    food
s    shells
g    guns (not gold)
p    planes
i    iron ore
d    dust (gold)
b    bars of gold
o    oil
l    light construction materials
h    heavy construction materials
r    radioactive materials
.FI

The delivery routes are indicated with \*Qgo-to\*Us
using the following codes, (assuming the sector in question is a mine):
.NF
.sp
 m/     up-right        (u in deliver)
 m>     right           (j in deliver)
 m\e     down-right      (n in deliver)
/m      down-left       (b in deliver)
<m      left            (g in deliver)
\em      up-left         (y in deliver)
etc.
.FI
.sp
An example:
.sp
.EX route f #
.NF
.eo
     - - - 0 0 0 0 0 0 0 0 0 0 1 1 1
     3 2 1 0 1 2 3 4 5 6 7 8 9 0 1 2
 -5         /a  /a              /a    -5
 -4       /k  <o              /g  <a  -4
 -3  a>  h> \j  \a           +        -3
 -2       \a  \a   a\     /a  /a      -2
 -1     /a   a\ /a   b  /+   +        -1
  0    m   c   u   t  \i              0
  1     \a  \l      \a  \a            1
  2        a/ \a   a/      a          2
     - - - 0 0 0 0 0 0 0 0 0 0 1 1 1
     3 2 1 0 1 2 3 4 5 6 7 8 9 0 1 2
.ec
.FI
.sp
.s1
.SA "deliver, cutoff, Distribution"
