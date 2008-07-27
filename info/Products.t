.TH Concept Products
.NA Products "Formulas for the production of commodities"
.LV Basic
.s1
Many Empire sector types exist specifically
to turn \*Qraw\*U materials into products.
Some of those products may, in turn,
be considered raw materials for other industries
which produce other products.
.s1
The show command displays characteristics of products:
.s1
.EX show product
.NF
product    cost  raw materials  reso dep  level p.e.
   iron i  $0                    min   0  1.0
   dust d  $0                   gold  20  1.0
   food f  $0                   fert   0  (tech+10)/(tech+20)
    oil o  $0                  ocont  10  (tech+10)/(tech+20)
    rad r  $2                   uran  35  (tech-40)/(tech-30)
 shells s  $3     2l  1h                  (tech-20)/(tech-10)
   guns g  $30    1o  5l 10h              (tech-20)/(tech-10)
 petrol p  $1     1o                      (tech-20)/(tech-10)
   bars b  $10    5d                      1.0
    lcm l  $0     1i                      (tech+10)/(tech+20)
    hcm h  $0     2i                      (tech+10)/(tech+20)
   tech    $300   1d  5o 10l              (educ-5)/(educ+5)
medical    $90    1d  5o 10l              (educ-5)/(educ+5)
    edu    $9     1l                      1.0
  happy    $9     1l                      1.0
.FI
.s1
The columns are:
.in \w'raw materials\0\0'u
.L product
The name of the product and the one-letter mnemonic of the commodity
made, if any.
.L cost
Cost per unit of production.
.L "raw materials"
Raw materials for one unit of production.
.L "reso dep"
Natural resource exploited, and its rate of depletion.
.L "level p.e."
Level production efficiency, for 100% sector type production
efficiency
.in
.s1
How much a sector can produce
is governed by the amount of \*Qwork\*U performed by its populace,
divided by the amount of raw materials
per unit of production.
E.g., it takes 3 units of \*Qwork\*U to produce 1 shell;
2 units to incorporate the light construction materials
and 1 unit for the heavy.
Mining a \*Qnatural resource\*U such as fertility
takes one unit of work;
e.g., it takes 1 unit of work per unit of food produced.
.s1
How many products each unit of production makes depends on the level
production efficiency, which is the product's level p.e. (shown by
"show product") times the sector type's p.e. (shown by "show sect
stats").  Production requires a level p.e. above zero, obviously.
.s1
Say your country is at tech level 25.  Then your agribusinesses' level
p.e. is (25+10)/(25+20) * 900% = 7, i.e. they'll make 7 food per unit
of production.  Your shell industries' level p.e. is only
(25-20)/(25-10) * 100% = 1/3, i.e. they'll make 1 shell per three
units of production.  Your uranium mines can't produce at all, because
their level p.e. is negative.
.s1
.SA "Producing, Updates, Item-types"
