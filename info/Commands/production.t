.TH Command PRODUCTION
.NA production "Predicts production of sectors at the next update"
.LV Basic
.SY "production <SECTS>"
Produce predicts how much is going to be produced at the next update.
It also shows you how much could be produced if more materials were
available in the sector.
.s1
Note that due to random rounding factors in the update, prod is not
totally accurate -- it may be off by a little bit.
.s1
In the syntax of the produce command
<SECTS> is the area on which you wish information,
(see \*Qinfo syntax\*U).
Produce reports on any sector that would produce goods,
sectors that don't produce anything
or are less than 60% efficient are ignored.
.s1
A typical usage might be:
.EX produce 0:9,0:9 ?type=m
which would list data for mines
in an area to the south-east of the capital.
.s1
The production report is headed by the lines:
.NF
PRODUCTION SIMULATION
   sect  eff des avail  make p.e. cost   use1 use2 use3  max1 max2 max3   max
.FI
These columns represent:
.s1
.in \w'max[123]\0\0'u
.L sect
the x and y coordinates of the sector
.L des
The sectors designation
.L eff
the efficiency of the sector in doing its task.
Note that some sectors although working at 100%
may show up less as the natural resources influence this figure.
.L avail
the amount of work done in the sector.
.L "make"
How much of what product will be made at the next update.
.L p.e.
Production efficiency \- the efficiency with which the product is made
(i.e. how much benefit education or technology gives).
.L cost
The cost of the product in dollars
.L use[123]
The three use fields show how much of what product are consumed in
producing the given number of products produced.
.L max[123]
The three max fields show what would be used if it were available in
the sector (i.e. this is what the workforce is capable of using if it
was available).
.L max
This field gives the maximum output for the workforce if all the raw
materials were present.
.in
.s1
For example:
.EX prod #
.NF
Fri Mar 18 20:58:55 2005
PRODUCTION SIMULATION
   sect  eff des avail  make p.e. cost   use1 use2 use3  max1 max2 max3   max
   4,-4   a 120%   600  999f 6.97 $0                                      999
   2,-4   t 100%   445 16.01 0.80 $6000   20d 100o 200l   28d 140o 280l 22.41
   1,-3   j 100%   600  465l 0.77 $0     600i            600i             465
   3,-3   g   0%   603    0d 1.00 $0                                        0
   4,-2   g   0%   602    0d 1.00 $0                                        0
   6,-2   j 100%   600  133l 0.77 $0     172i            600i             465
   0,-2   e 100%   395  153m 1.00 $459   153c            153c             280
   2,-2   b 100%   599  120b 1.00 $1200  600d            600d             120
   5,-1   g   0%     2    0d 1.00 $0                                        0
   7,-1   i 100%   600   60s 0.30 $600   400l 200h       400l 200h         60
   3,-1   g   0%     0    0d 1.00 $0                                        0
   4,0    g   0%     4    0d 1.00 $0                                        0
   6,0    g  69%   501  346d 1.00 $0                                      346
   8,0    k 100%   600    0h 0.77 $0       0i            600i             232
   0,0    m 100%   810  810i 1.00 $0                                      810
   2,0    m 100%   743  743i 1.00 $0                                      743
   5,1    g   0%     2    0d 1.00 $0                                        0
  -1,1    l 100%   600  200  1.00 $1800  200l            600l             600
   1,1    g   0%     0    0d 1.00 $0                                        0
   3,1    m 100%   600  600i 1.00 $0                                      600
   4,2    g  65%   501  326d 1.00 $0                                      326
  -2,2    j 100%   600  465l 0.77 $0     600i            600i             465
   0,2    k 100%   600  232h 0.77 $0     600i            600i             232
   2,2    d 100%   600   11g 0.30 $1110   37o 185l 370h   38o 190l 380h    11
   3,3    k 100%   600  232h 0.77 $0     600i            600i             232
  10,4    o  22%   600  102o 0.77 $0                                      102
26 sectors
.FI
.s1
.SA "Products, Update, Production"
