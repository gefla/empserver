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
 sect  des eff wkfc will make--- p.e. cost use1 use2 use3  max1 max2 max3   max
.FI
These columns represent:
.s1
.in \w'will make---\0\0'u
.L sect
the x and y coordinates of the sector
.L des
The sectors designation
.L eff
the efficiency of the sector in doing its task.
Note that some sectors although working at 100%
may show up less as the natural resources influence this figure.
.L wkfc
the number of active workers in the sector (in units of civilians).
.L will make---
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
.in \\n(in
.s1
For example:
.EX prod #4 ?civ>100
.NF
prod #
Wed Aug 17 08:28:21 1988
PRODUCTION SIMULATION
 sect  des eff wkfc will make--- p.e. cost use1 use2 use3  max1 max2 max3   max
  1,-3  m  56% 1003   22 iron    1.00 $0                                    22
  5,-3  a  46% 1006  171 food    0.87 $0                                   171
  9,-3  l 100% 1003    0 edu     1.00 $0     0l             40l             40
-14,-2  r 100%  999 0.00 medical 0.06 $0     0d   0o   0l    2d  10o  20l 0.12
 -8,-2  j 100%  798   32 lcm     0.87 $0    32i             32i             32
 -6,-2  o   1%  182    0 oil     0.87 $0                                     0
 -4,-2  a 100% 1003  360 food    0.87 $0                                   360
  0,-2  m  60% 1003   24 iron    1.00 $0                                    24
  4,-2  m  54%  999   22 iron    1.00 $0                                    22
  8,-2  % 100%  999    0 petrol  0.78 $0     0o             40o            400
-15,-1  l 100% 1003   26 edu     1.00 $234  26l             40l             40
-13,-1  o  28%  968   11 oil     0.87 $0                                    11
 -3,-1  m  51% 1003   20 iron    1.00 $0                                    20
  5,-1  k 100% 1998   40 hcm     0.87 $0    78i             78i             40
-28,0   a  53% 1003   19 food    0.87 $0                                    19
-16,0   m  62% 1003   25 iron    1.00 $0                                    25
-12,0   j 100%  999   40 lcm     0.87 $0    39i             39i             40
 -6,0   m  50%  964   19 iron    1.00 $0                                    19
 -4,0   m  59%  935   22 iron    1.00 $0                                    22
    19 sectors
.FI
.s1
.SA "Products, Update, Production"
