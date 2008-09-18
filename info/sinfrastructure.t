.TH Command SINFRASTRUCTURE
.NA sinfrastructure "Report infrastructure of sectors"
.LV Basic
.SY "sinfrastructure <SECTS>"
The sinfrastructure command displays specific infrastructure information on
some or all of the sectors you occupy.
.s1
In the syntax of the census command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
.s1
A typical usage might be:
.EX sinfrastructure 0:9,0:9 ?des=m
which would list data for mines in an area
to the south-east of the capital.
.s1
A sinfrastructure lists each of your sectors in the specified area headed by:
.NF
Thu Aug  8 08:37:03 1996
                      road        rail       defense
  sect        eff   eff mcost   eff mcost   eff  fact
.FI
These columns represent:
.s1
.in \w'defense fact\0\0'u
.L sect
the x and y coordinates of the sector
.L eff
the efficiency of the sector (affects all benefits
of a designated sector except mobility and defense)
.L "road eff"
the road efficiency of the sector
.L "road mcost"
the mcost used to determine the mobility cost of moving things and marching
non-train units through the sector
.L "rail eff"
the rail efficiency of the sector
.L "rail mcost"
the mcost used to determine the mobility cost of moving trains through the
sector.  If the rail eff is 0, trains cannot move through this sector.
.L "defense eff"
the defense efficiency of the sector.
.L "defense fact"
the defense factor of the sector.  This is how well the sector defends.
All sectors start at 1, and improve (see "improve") from there to the maximum
shown in "show sect stats"
.in
.s1
For example:
.EX sinf #0
.NF
Thu Aug  8 08:37:03 1996
                      road        rail       defense
  sect        eff   eff mcost   eff mcost   eff  fact
   1,-1   f    0%    0% 0.400    0% 0.400    6%  1.18
   0,0    c  100%    2% 0.192   28% 0.113    8%  1.08
   2,0    m  100%    0% 0.200    0% 0.200  100%  2.00
3 sectors
.FI
.s1
.SA "improve, census, commodity, resource, Sectors, Infrastructure, Mobility, Commodities"
