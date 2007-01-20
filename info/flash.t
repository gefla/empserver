.TH Command FLASH
.NA flash "Send a message directly to a friend's terminal"
.LV Expert
.SY "flash <CNUM/CNAME> [<MESSAGE>]"
.s1
The <CNUM/CNAME> argument is either the number or name of
the recipient nation.
.s1
flash allows instant communication between friendly nations.
flash has two different syntaxes.  You can either send your friend a one
line message as in:
.EX flash Foobots Hi there, I'm back from lunch
.s1
Or you can send a multi-line message:
.EX flash Foobots
.NF
> Hi there.
> I ate Fodderland for lunch.
> .
.FI
Note that you can end a multi-line flash with either a '.' (period)
on a line all by itself, or by hitting ^D on a line by itself.
.s1
If you do not wish to receive any flash messages from your friends,
then you can temporarily turn the feature off by typing "toggle flash".
.s1
Note that if the person you are flashing is allied to you, you will
be informed if they are not logged in or not accepting flashes when you attempt
to flash them.  This only works when *they* are allied with you, since
you would normally see them logged in or not using "players"
.s1
.SA "toggle, wall, declare, players, telegram, Communication"
