.TH Command WAIT
.NA wait "Wait for a telegram"
.LV Expert
.SY "wait"
The wait command is used to wait until new telegrams are received.
It's most common use is to wait for an attack, or a reply of some sort.
.s1
.EX "wait"
.nf
You have a new telegram waiting ...
.fi
.EX
.sp
When telegrams arrive you are informed with:
.NF
\*QYou have a new telegram waiting ...\*U   if there is just one new one, or
\*QYou have ten new telegrams waiting ...\*U   if there are ten of them, or
\*QYou have several new telegrams waiting ...\*U   if there are very many.
.FI
.s1
The wait command will eventually time out.
.s1
WAIT COMMAND IS DISABLED IN THE NEW SERVER.
.s1
.SA "telegram, wire, Obsolete"
