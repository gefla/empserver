.TH Command PLAYERS
.NA players "List players currently connected to the game"
.LV Expert
.SY "players"
.EX players
.NF
Fri Sep 29 17:27:42 1995
            #                                   time  idle last command
Khazad'ur  34             tetherow@nol.nol.org  0:38   32s cen 0,0
children    0    children@RedDragon.Empire.Net  0:16    7s play
2 players
.FI
.L "#"
the country number of the player.
.L time
the total number of hours:minutes the player has been connected today
.L idle
the number of seconds since the player has typed their last command
.L "last command"
the last command that the player typed
.s1
If you are not a deity, then the "players" command will only list
your allies (and omit userid@host.name).  Also,
non-deity players will only be given an estimate (based on a secret
random formula) of the total number of players currently logged on.
.s1
.SA "country, Nation, Communication, Deity, Playing"
