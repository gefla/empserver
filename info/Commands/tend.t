.TH Command TEND
.NA tend "Transfer commodities between ships"
.LV Basic
.SY "tend <COMMODITY|'land'> <SHIP> <AMT|UNIT> <to-SHIP>"
The tend command allows transfer of commodities
from a particular (supply) ship to patrol boats,
submarines, battleships, etc. while at sea.
.s1
If you had cargo ship #16 which had food, and your fleet S needed food,
you could resupply the entire fleet with 20 units of food by typing
.EX tend food 16 20 S
.s1
Requirements are that the tending ship must be in the same sector
as the tended ships, and it must be yours, or belong to a country that
considers you a friendly trading partner.
A positive <AMT> indicates the amount the tender will give away,
while a negative amount indicates the amount that the tender
will try to extract from each ship. (However, you cannot take goods
from a friendly ship by giving a negative amount)
.s1
Only land units with the "assault" capability may be tended between
ships.  If you wanted to tend land unit #23 from ship #8 to ship #17,
then you would type:
.EX tend land 8 23 17
Besides the "assault" capability restriction, tending land units has
the same effect as though you were simply loading the unit onto a
ship.  In particular, you may tend land units to "friendly" ships in
the same sector.
.s1
.SA "load, unload, lload, lunload, ltend, Ships"
