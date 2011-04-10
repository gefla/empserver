.TH Command SHAREBMAP
.NA sharebmap "Share parts of your bmap with another country"
.LV Expert
.SY "sharebmap <CNUM/CNAME> <SECTS> [<des>]"
This command will share all of the information on your bmap with the
country <CNUM/CNAME>.  In order for it to work, they must be friendly
towards you and your bmaps must already overlap to some extent.  Note
that your 'working' bmaps are not compared for this, but rather your 'true' bmaps.  In particular, you can not fool the game into thinking
that your bmaps overlap through using the 'bdes' command.  See the
bottom of 'info bmap' for an explanation of the difference between
your 'working' and your 'true' bmap.
.s1
When sectors owned by you get transferred to your friend's bmap, they
will be replaced with the first letter of your country name
(capitalized), the only exception being harbors and bridges which will
always be transmitted.  Also, if you want to send them the location of
owned sectors having a certain designation you can specify that using
the optional <des> argument.  For example if your country name was
"TheScum" and you typed:
.EX "sharebmap Fodderland #1 j"
then all of your bmap would be added to Fodderland's bmap, with sectors
owned by you replaced with 'T', except for your '=', 'h', and 'j'
sectors which would be sent as is.
.s1
Note that the destination bmap will only be changed if in that
location there was a '?', a '.', a ' ', or the first letter
of your country name (capitalized).  Thus, the receiving country will
not lose any information in this exchange.
.s1
Note that the sharebmap command only changes your 'working' bmap and
so you can delete sharebmap information from your bmap by typing:
.EX "bmap <SECTS> revert"
See 'info bmap' for more details.
.s1
.SA "bdes, bmap, Communication, Maps"
