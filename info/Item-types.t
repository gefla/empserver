.TH Concept "Item types"
.NA Item-types "Description of the different types of items"
.LV Basic
.s1
The different kinds of people and materials are called items, or
sometimes commodities.
.s1
The show command displays the detailed characteristics of items.
.s1
.EX show item
.NF
item value sell lbs    packing     melt  item
mnem                in no wh ur bk deno  name
   c     1   no   1  1 10 10 10 10    4  civilians
   m     0   no   1  1  1  1  1  1   20  military
   s     5  yes   1  1  1 10  1  1   80  shells
   g    60  yes  10  1  1 10  1  1  100  guns
   p     4  yes   1  1  1 10  1  1   50  petrol
   i     2  yes   1  1  1 10  1  1  100  iron ore
   d    20  yes   5  1  1 10  1  1  100  dust (gold)
   b   280  yes  50  1  1  5  1  4  200  bars of gold
   f     0  yes   1  1  1 10  1  1    2  food
   o     8  yes   1  1  1 10  1  1   50  oil
   l     2  yes   1  1  1 10  1  1  100  light products
   h     4  yes   1  1  1 10  1  1  100  heavy products
   u     1  yes   2  1  1  2  1  1    2  uncompensated workers
   r   150  yes   8  1  1 10  1  1 1000  radioactive materials
.FI
.s1
The meaning of the headings are:
.in \w'packing wh\0\0'u
.L "item mnem"
A one-letter mnemonic abbreviation of the item name.  Commands let you
use this to select the item.
.L value
The value if the item is mortgaged.
.L sell
Whether you can sell the item on the market.
.L lbs
The weight of the item, which determines how much mobility it takes to
move it.
.L "packing in"
The packing bonus the item receives in inefficient (<60%) sectors.
.L "packing no"
The packing bonus the item receives in normal sectors.
.L "packing wh"
The packing bonus the item receives in warehouse sectors.
.L "packing ur"
The packing bonus the item receives in urban sectors.
.L "packing bk"
The packing bonus the item receives in bank sectors.
.L "item name"
The full name of the item.
.in
.s1
To find out which sectors are in what packing groups, try the "show
sector s" command.
.s1
.SA "Commodities, Products, show, Sector-types, Producing"
