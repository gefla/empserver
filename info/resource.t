.TH Command RESOURCE
.NA resource "Show the resources of sectors"
.LV Basic
.SY "resource <SECTS>"
The resource command displays specific information on
some or all of the sectors you occupy.
.s1
In the syntax of the resource command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
.s1
A typical usage might be:
.EX resource 0:9,0:9 ?des=m
which would list data for mines in an area
to the south-east of the capital.
.s1
A resource report lists each of your sectors in the specified area
headed by:
.NF
RESOURCE
  sect        eff  min gold fert oil uran ter
.FI
These columns represent:
.s1
.in \w'uran\0\0'u
.L sect
the x and y coordinates of the sector
.L eff
the efficiency of the sector (affects all benefits
of a designated sector except mobility units)
.L min
the percentage richness of non-gold minerals,
(range is 0-100, not used up, i.e. a renewable resource)
.L gold
the amount of gold ore,
(range is 0-100, in absolute tons, i.e. non-renewable)
.L fert
the fertility of the soil in relative terms,
(range is 0-100, not used up, i.e. a renewable resource)
.L oil
the petroleum content of the soil in relative terms,
(range is 0-100, in absolute tons, i.e. non-renewable)
.L uran
the uranium (radioactive materials) content in relative terms,
(non-renewable)
.L ter
the territory number.
.in
.s1
For example:
.EX reso #0
.NF
Sat Sep  2 03:20:00 1989
RESOURCE
  sect        eff  min gold fert oil uran ter
   3,-5   ^   40%   38    0   64   0   44
   5,-5   )  100%   75    0   41   0   36  90
   7,-5   t  100%   65    0   38   0   72
   9,-5   h  100%   48    0   42   0   45   5
   0,-4   j  100%    0    0  112  40    0
   2,-4   j  100%   32    0   69   0   36
   4,-4   l  100%   60    0   53   0   37  22
   6,-4   +  100%   52    0   14   0   78
  10,-4   ^   20%  100    0    0   0   79
    9 sectors
.FI
.s1
.SA "commodity, census, Products, Sectors"
