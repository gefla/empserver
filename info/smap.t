.TH Command SMAP
.NA smap "Get a map around a ship"
.LV Basic
.SY "map <SECTS | SHIP> <s|p|l|*|h>"
.SY "nmap <SECTS | SHIP> <s|p|l|*|h>"
.SY "smap <SECTS | SHIP> <s|p|l|*|h>"
.SY "lmap <SECTS | LAND UNIT> <s|p|l|*|h>"
.SY "pmap <SECTS | PLANE> <s|p|l|*|h>"
A map gives you a graphic representation of all or part of your country.
.s1
Your own sectors as well as sea, mountains, wasteland and unowned
wilderness and plains show up as a designation mnemonic (see
\*QSector-types\*U).  Other sectors appear as question marks
(\*Q?\*U).
.s1
You can also get this by typing 'M' at the navigation prompt.
.s1
Examples:
.EX map -9:18,-8:5
generates a 28 x 14 map based on data supplied by the sectors
in the area specified.
.NF
    ---------0000000000111111111
    9876543210123456789012345678
 -8        . . . - -             -8
 -7       . . . . - ^            -7
 -6    . . . . - - - -           -6
 -5   . . - . a a - ^ ? ? -      -5
 -4  . . . . k o ! - ^ ? ? ?     -4
 -3 . - . a h j a ^ ^ ^ ^ ^ -    -3
 -2  . . - . a a a ^ a a - - -   -2
 -1   . . . a a a b + + ^ ^ -    -1
  0  . . . m c u a a - ^ - ^     0
  1   . - . a a . a a - - ^      1
  2    - ^ ^ a a a ^ a - - ^     2
  3     - - - - - - - . . -      3
  4      ^ - - - - - ^ - -       4
  5       - ^ - - ^ - - -        5
    ---------0000000000111111111
    9876543210123456789012345678
.FI
.s1
.EX map # >mapfil
where your \*Qrealm\*U (or '#',
see \*Qinfo realm\*U or \*Qinfo update\*U)
is -5:5,-6:6 will type out a 11 by 13 sector map
and also put the map in the file called \*Qmapfil\*U.
.s1
Also, if you give a ship # instead of coordinates or a realm, map
will show you a small realm around the given ship (assuming you
own it).
.s1
If you use nmap, you can get a map that shows new sector designations
instead of old sector designations of your own country.
.s1
If you use lmap, you can get a map around a land unit, 
instead. Pmap does the same for a plane. Smap does
the same for a ship.
.s1
.L "MAP FLAGS"
.s1
If you give an 's' flag, all your ships will be shown on the map.
An 'l' flag does the same for land units, and a 'p' for planes.
\&'*' shows all in this order: land units, ships, planes.
To highlight sectors you own, specifiy the 'h' flag.
.s1
.SA "census, commodity, radar, realm, route, update, Ships, Maps"
