.TH Command REALM
.NA realm "Print out or modify one of your \*Qrealms\*U"
.LV Basic
.SY "realm <NUMBER> <SECTS>"
The realm command allows you to manipulate the contents of the
\*Qrealm\*Us associated with your country
which provide a convenient way to save coordinates that are
frequently used
(see \*Qinfo syntax\*U for use of realm arguments).
.s1
In the syntax of the realm command
<NUMBER> is the number of the realm to be set.
.s1
The <SECTS> argument is optional;
if included, the specified realm will be set to <SECTS>;
if omitted the coordinates of the specified realm will be printed.
.sp
.EX realm 1
.NF
Realm 1 is -3:3,3:3
.FI
.EX realm 1 -3:2,4
.EX realm 1
.NF
Realm 1 is now set to -3:2,4:4
.FI
.s1
If the <NUMBER> argument is omitted,
the coordinates of all realms will be printed.
.s1
Also note that the <SECTS> argument may use the \*Q#\*U form of realm
specification.
.EX realm 0 #1
Now realm 0 is set to -3:2,4:4.
.s1
.SA "Syntax, nation, Maps"
