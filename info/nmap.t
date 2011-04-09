.TH Command NMAP
.NA nmap "Generate a map showing new sector types or around a nuke"
.LV Basic
.SY "nmap <SECTS|NUKE> [<MAP-FLAGS>]"
A map gives you a graphic representation of all or part of your country.
.s1
Your own sectors as well as sea, mountains, wasteland and unowned
wilderness and plains show up as a designation mnemonic (see
\*QSector-types\*U).  Other sectors appear as question marks
(\*Q?\*U).
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
If you use lmap, you can get a map around a land unit.
If you use pmap, you can get a map around a plane.
If you use smap, you can get a map around a ship.
If you use nmap, you can get a map around a nuke.
.s1
.L "MAP FLAGS"
.s1
Optional argument <MAP-FLAGS> consists of map flag characters.
If you specify an 's' flag, map will put your ships on the map.
If you specify an 'l' flag, map will put your land units on the map.
If you specify an 'p' flag, map will put your planes on the map.
If you specify an 'n' flag, map will put your nukes on the map.
A '*' will put all on the map.
When multiple unit types are selected they are displayed
in the following priority order: nukes, land units, ships, planes.
To highlight sectors you own, specifiy the 'h' flag.
.s1
.SA "census, commodity, radar, realm, route, Nukes, Maps"
