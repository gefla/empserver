.TH Command WALL
.NA wall "Send a message directly to all allies terminals"
.LV Expert
.SY "wall [<MESSAGE>]"
.s1
The wall command allows instant communication between allied nations.
It may be called in two different ways.  You can either send your ally a one
line message as in:
.EX wall Hi there, I'm back from lunch
.s1
Or you can send a multi-line message:
.EX wall
.NF
> Everyone ready?
> OK, let's hit the beaches!
> .
.FI
Note that you can end a multi-line wall with either a '.' (period)
on a line all by itself, or by hitting ^D on a line by itself.
Note also that all messages are truncated to 60 characters long!
.s1
If you do not wish to receive any broadcast messages from your allies,
then you can temporarily turn the feature off by typing "toggle flash".
.s1
.SA "flash, declare, telegram, Communication"
