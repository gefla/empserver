.TH Introduction Kill
.NA Kill "How to kill your process on the server"
.LV Basic
.s1
At some point you may lose your connection to the game.
When this happens its possible that the game will not
log your country off and you will get a message saying that
you are still playing the game.  This can be easily fixed by the player
if you type in the following.
.s1
.nf
.in +1i
telnet your.local.empire.machine 6666 
coun <NAME>  
pass <PASSWORD> 
kill
quit  
.in -1i
.fi
.s1
What happens is that you are connecting directly to the Empire Server.
From there you enter your country name and password.
You can then kill the job that's running and then quit so you can
login again using the client.
.s1
This little trick will work on just about any Empire game.
Of course you need to telnet to that game and
and enter the correct country and password.
.s1
Its a good idea to write this down somewhere and DON'T LOSE it cause
chances are that you will have to use it at sometime.
.s1
Note that many clients offer a "kill" command which does this.
.s1
.SA "Introduction"
