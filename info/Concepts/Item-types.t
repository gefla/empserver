.TH Concept "Item types"
.NA Item-types "Characteristics of Items"
.LV Basic
.s1
Items are the products produced in Empire.  Items are also known as
Commodities.
.s1
The show command displays the detailed characteristics of items.
.s1
.EX show item
.NF
Printing for tech level '344'
item   value sell lbs   packing   item
mnemo                 rg wh ur bk name
     c     1   no   1 10 10 10 10 civilians
     m     0   no   1  1  1  1  1 military
     s     5  yes   1  1 10  1  1 shells
     g    60  yes  10  1 10  1  1 guns
     p     4  yes   1  1 10  1  1 petrol
     i     2  yes   1  1 10  1  1 iron ore
     d    20  yes   5  1 10  1  1 dust (gold)
     b   280  yes  50  1  5  1  4 bars of gold
     f     0  yes   1  1 10  1  1 food
     o     8  yes   1  1 10  1  1 oil
     l     2  yes   1  1 10  1  1 light products
     h     4  yes   1  1 10  1  1 heavy products
     u     1  yes   2  1  2  1  1 uncompensated workers
     r   150  yes   8  1 10  1  1 radioactive materials
.FI
.s1
The meaning of the headings are:
.in \w'packing wh\0\0'u
.L "item mnemo"
A one-letter mnemonic abbreviation of the item name.  Commands let you
use this to select the item.
.L value
The value if the item is mortgaged.
.L sell
Whether you can sell the item on the market.
.L lbs
The weight of the item, which determines how much mobility it takes to
move it.
.L "packing rg"
The packing bonus the item receives in regular sectors.
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
.SA "Commodities, Products, show, Sector-types, Production"
