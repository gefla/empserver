.TH Command ECHO
.NA echo "Echo a string back to the client"
.LV Expert
.SY "echo [<string>]"
The echo command echos its arguments.
It is designed to help clients synchronize their commands
with the server when operating asynchronously.  Note that if there are
any spaces in your string, then you will need to include the string in
quotes.
.s1
For example:
.EX echo SN#123456
.nf
SN#123456
.fi
.EX echo hi mom
.nf
hi
.fi
.EX echo \&"hi mom"
.nf
hi mom
.fi
.SA "toggle, Communication"
