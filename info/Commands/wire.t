.TH Command WIRE
.NA wire "Read announcements"
.LV Basic
.SY "wire [yes|no|<days>]"
The wire command is used to read the announcements sent you by other countries.
.sp
After reading all your announcements you may mark them
as being read by 
specifying \*Qyes\*U or not by specifying \*Qno\*U as an argument.
If an argument is not supplied then you will be prompted
to either answer \*Qyes\*U or \*Qno\*U to marking announcements
as being read.
If you supply or answer \*Qno\*U, the announcements will remain
pending but you will not be reminded of them.
.sp
You can read announcements that you have previously marked as read by
specifying the <days> argument.  For example:
.EX "wire 3"
would print the announcements from the last three days.
If you specify <days> argument, you will not be prompted to
mark the announcements as being read.
.s1
In order to read telegrams, use the 'read' command.
.s1
.SA "telegram, read, accept, reject, Communication"
