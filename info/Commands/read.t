.TH Command READ
.NA read "Read your telegrams"
.LV Basic
.SY "read [yes|no|<CNUM/CNAME>]"
The read command is used to read the telegrams sent you
by other countries and by the game,
(e.g. when you make a sale or are attacked).
.sp
After reading all your telegrams you may delete them by 
specifying \*Qyes\*U or not by specifying \*Qno\*U as an argument.
If an argument is not supplied then you will be prompted
to either answer \*Qyes\*U or \*Qno\*U to delete the telegrams.
If you supply or answer \*Qno\*U, the telegrams will remain
pending but you will not be reminded of them.
.sp
Deities can specify a <CNUM/CNAME> argument which
selects a country's telegrams to read.
If the <CNUM/CNAME> argument is supplied, the telegrams can not be deleted.
.sp
When telegrams arrive you are informed with:
.NF
\*QYou have a new telegram waiting ...\*U   if there is just one new one, or
\*QYou have ten new telegrams waiting ...\*U   if there are ten of them, or
\*QYou have several new telegrams waiting ...\*U   if there are very many.
.FI
.s1
This command reads only
telegrams & bulletins, use the 'wire' command to read announcements.
.s1
.SA "toggle, telegram, wire, flash, accept, reject, Communication"
