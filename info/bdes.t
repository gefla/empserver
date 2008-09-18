.TH Command BDES
.NA bdes "Manually change your bmap"
.LV Expert
.SY "bdes <SECT> <des>"
A bmap gives you a graphic representation of all or part of your country.
Bdes allows you to change a sector of your bmap.
.s1
Bmap differs from map in that it reflects everything you've learned
about the world. If, for example, you navigate near a sector and find
out that it is as sea, it will show up on your bmap as sea, even if it
doesn't show up on your map.
.s1
When you get new information, it will overwrite the designation you put in
with bdes, UNLESS you put in an 'x' or an 'X'. These two characters, 'x'
and 'X' are permanent, unless you manually change them with bdes. Once you
change them back, the sector will be updated normally by nav, march, etc.
(The normal use for these is to record minefields, so that you can march
adjacent to the minefield without eliminating the information)
.s1
Unlike normal designation, with bdes, you may use any character.
If you wish to use a space, surround it with double quotes:
bdes # " "
.s1
You can't bdesignate something to a ?
.s1
Conditionals do not work with bdes (i.e. bdes # ?des=c C) unless
you own the sector.
.s1
Note that the bdes command only changes your 'working' bmap and not
your 'true' bmap.  If you make a mistake and bdes a bunch of sectors
where you didn't want them to be, you can always start over by typing
.EX "bmap revert"
which will revert your 'working' bmap back to your 'true' bmap.
.s1
.SA "bmap, sharebmap, Maps"
