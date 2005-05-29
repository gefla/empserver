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
This table gives the constituents and costs for each type of product.
.s1
NOTE: All costs given in the table are for 100% production efficiency.
Many processes depend on technology level
or educational level of the country;
a note like \*Qp.e.=(tlev+10)/(tlev+20)\*U means
that a country with a technology level of 0
has a production efficiency of 50%
and will only produce one-half of the specified product amount
for the given costs,
whereas a country with a technology level of 100
will have a production efficiency of 92%.
.s1
In addition, some sectors don't produce raw materials as well as others,
and some produce much better than others.  Use the "show sector capabilities"
command to see production efficiencies and other useful information on
producing sectors.
.s1
The rate at which constituents are turned into products
is governed by the \*Qwork\*U accrued to a sector
(see \*Qinfo Innards\*U) divided by the units of constituents
per unit of product.
E.g., it takes 3 units of \*Qwork\*U to produce 1 shell;
2 units to incorporate the light construction materials
and 1 unit for the heavy.
\*QNatural resources\*U such as fertility
count as one unit of constituents;
e.g., it takes 1 unit of work per unit of food produced.
.s1
.in \w'technology\0\0'u
.L shells
Shell production efficiency depends on technology level;
p.e.=(tlev-20)/(tlev-10)
Shells cost $3 each to manufacture.
Each requires 2 units of light construction materials
and 1 unit of heavy construction materials.
You must have a minimum technology of 20 to produce shells.
.L guns
Gun production depends on technology level;
p.e.=(tlev-20)/(tlev-10)
Guns cost $30 each to manufacture.
Each requires 5 units of light construction materials,
10 units of heavy construction materials,
and 1 unit of oil.
You must have a minimum technology of 20 to produce guns.
.L iron
Iron ore production depends on mineral content in mines.
Extracting iron ore is free,
and the mineral content of the sector is not depleted
by mining iron.
.L dust
Gold dust production depends on gold mineral content in gold mines.
Extracting gold dust is free,
but the gold mineral content of the sector is depleted
by mining gold dust.  In addition, mountains may sometimes produce
gold dust as well.  The efficiency of mountains producing gold dust
may not be as good as gold mines.  Use the "show sector capabilities" 
command to see the production efficiency of sectors.
.L bars
Gold bars are produced in banks.
Gold bar production depends on gold dust in gold mines.
Gold bars cost $10 to produce and require 5 units of gold dust each.
.L food
Food production efficiency depends on technology level
and the fertility of the sector;
p.e.=(tlev+10)/(tlev+20).
Growing food is free,
and the fertility of the sector is not depleted by growing food.
.L oil
Oil production efficiency depends on technology level
and the oil content of the sector;
p.e.=(tlev+10)/(tlev+20).
Pumping oil is free,
but the oil content of the sector is depleted
by extracting and refining oil.
.L petroleum
Petroleum production depends on technology level;
p.e.=(tlev-20)/(tlev-10).
A unit of petroleum costs $1 to manufacture.
Every 10 units of petroleum require 1 unit of oil to produce.
You must have a minimum technology of 20 to produce petroleum.
.L lcm
Light construction material production efficiency
depends on technology level;
p.e.=(tlev+10)/(tlev+20).
Producing lcms does not cost money, but requires 1 unit of iron ore. 
.L hcm
Heavy construction material production efficiency
depends on technology level;
p.e.=(tlev+10)/(tlev+20).
Producing hcms does not cost money, but requires 2 unit of iron ore. 
.L rad
Radioactive material production efficiency depends on technology level,
which must be at least 40.
p.e.=(tlev-40)/(tlev-30).
Producing rads cost $2 per ton,
and depletes the uranium content of the sector.
.L education
A unit of education (a \*Qclass of graduates\*U),
costs $10 to produce and requires 1 unit
of light construction materials (see "info Education").
.L happiness
A unit of happiness (\*Qhappy strollers\*U),
costs $10 to produce and requires 1 unit
of light construction materials (see "info Happiness").
.L technology
Technology production efficiency depends on education level;
p.e.=(elev-5)/(elev+5).
A unit of technology (a \*Qtechnological breakthrough\*U),
costs $5 times the number of ETUs per update
to produce, and requires 10 units
of light construction materials,
5 units of oil and 1 unit of gold dust.  For more details about how
technology level is calculated from technology units, see "info Technology".
.L research
Research production efficiency depends on education level;
p.e.=(elev-5)/(elev+5).
A unit of research (a \*Qmedical discovery\*U),
costs $90 to produce and requires 10 units
of light construction materials,
5 units of oil and 1 unit of gold dust (see "info Research").
.in
.SA "Producing, Updates"
