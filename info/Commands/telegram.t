.TH Command TELEGRAM
.NA telegram "Send \*Qdiplomatic\*U communique to another country"
.LV Basic
.SY "telegram <CNUM/CNAME> [<CNUM/CNAME>] [[<CNUM/CNAME>]]"
The telegram command allows non-treaty communication
to take place between representatives.  Currently you
are given only 1024 characters per telegram.
.s1
The <CNUM/CNAME> argument is either the number or name of
the recipient nation. You may specify as many recipients on the
command line as you wish.
Telegrams sent to country #0 appear as prayers,
(favors are best obtained in this manner.)
.s1
While entering your telegram the system tells you how many characters
you have left.  You end the telegram by typing ^D on a line by itself,
or by entering a line with a '.' on a line by itself. A line containing
only a '~' will abort the telegram.
.s1
For example:
.EX tel 7 8
.NF
Enter telegram for multiple recipients; end with ^D
 1024 left:
 1023 left: B.F.D. Pouncetrifle
 1003 left: Groonland Embassy
  984 left:
  983 left: Dear Ambassador Pouncetrifle,
  953 left:
  952 left: Just a little note to express the friendly wishes that all of us
  887 left: in Curmudgeon feel toward you backward savages in Groonland.  As
  822 left: to the matter of the treaty you've proposed please remember that
  757 left: if we wished we could TRASH your capital.
  715 left:
  714 left:                         As always I remain,
  688 left:                         Your humble and obedient servant, etc, etc
  638 left:                         E. D. Amen Dada
  616 left:                         Under Asst. West Coast Promo.
  580 left:                         the Sovereign State of Curmudgeon
  540 left: <EOT>
.FI
.s1
Note that others can reject your telegrams, using \*Qreject\*U.
.s1
.SA "flash, toggle, announce, read, reject, Communication, Diplomacy"
