.TH Command SBMAP
.NA bmap "Big map of all your worldly knowledge around a ship"
.LV Basic
.SY "bmap [<SECTS> | <SHIP>] <s|p|l|r|t|*|h>"
.SY "lbmap [<SECTS> | <LAND UNIT>] <s|p|l|*|h>"
.SY "pbmap [<SECTS> | <PLANE>] <s|p|l|*|h>"
.SY "sbmap [<SECTS> | <SHIP>] <s|p|l|*|h>"
A bmap gives you a graphic representation of all or part of your country.
.s1
Bmap differs from map in that it reflects everything you've learned
about the world. If, for example, you navigate near a sector and find
out that it is sea, it will show up on your bmap as sea, even if it 
doesn't show up on your map.
.s1
Note that bmap shows the \*QLAST KNOWN INFORMATION\*U you have about
a sector. It may change, and your bmap will not reflect it. For example,
if you re-designate a sector, it won't show up on your bmap as the
new sector type until you do a map.
.s1
Commands contributing to bmap include (but are not limited to):
map, nav, fly, bomb, recon, para, drop, lookout, coastwatch, radar.
(probably others)
.s1
Your own sectors show up as a designation mnemonic
(see \*QSector-types\*U) while sectors held by other countries
appear as question marks (\*Q?\*U).
Mountains, wilderness areas, and seas appear
as '^', '-' and '.' respectively.
.s1
You can also get an lbmap by typing 'B' at the navigation prompt.
.s1
Note that any sector
marked as 'X' on your bmap will automatically be avoided by
ships when they are trying to calculate the best path between two
points.  Whenever you detect a sea mine in a sector, the server will
automatically put an 'X' on your bmap.
.s1
You can call up a local bmap while navigating or marching by typing 'B'
at the prompt.
.s1
Examples:
.EX bmap -9:18,-8:5
generates a 28 x 14 bmap based on data supplied by the sectors
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
.EX bmap # >mapfil
where your \*Qrealm\*U (or '#',
see \*Qinfo realm\*U or \*Qinfo update\*U)
is -5:5,-6:6 will type out a 11 by 13 sector bmap
and also put the bmap in the file called \*Qmapfil\*U.
.s1
You can also give the number of a ship, and bmap will give you a small
bmap centered on the location of that ship, provided you own it.
If you use lmap, the number of a land unit will give you a map around
that land unit. The pmap command does the same for planes.
.s1
.L "BMAP FLAGS"
.s1
If you specify an 's' flag, bmap will put your ships on the map.
If you specify an 'l' flag, bmap will put your land units on the map.
If you specify an 'p' flag, bmap will put your planes on the map.
A '*' will put all on the map, in this order: land units, ships, planes.
To highlight sectors you own, specify the 'h' flag.
.s1
You actually have two bmaps.  One which is created by the server; this
one is called your 'true' bmap.  You can view your true bmap at any
time using the
.SY "bmap <SECTS> true"
command.  However, usually when you view your bmap, you are looking at
your 'working' bmap.  The only difference between your working bmap
and your true bmap is that your working bmap also contains
designations put on there by you using the 'bdes' command, and by your
friends using the "sharebmap" command.  If somehow your bmap gets
corrupted, you can revert your working bmap back to your true bmap
using the command:
.SY "bmap <SECTS> revert"
.s1
.SA "bdes, map, navigate, fly, bomb, recon, paradrop, drop, lookout, coastwatch, radar, sharebmap, Ships, Maps"
