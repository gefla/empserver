.TH Command ZDONE
.NA zdone "Indicate willingness to have an update"
.LV Expert
.SY "zdone [yes|no|check]"
The zdone command allows you to say when you are done and ready
for an update.
.s1
"yes" says that you are ready for an update.
.s1
"no" clears any previous "yes" that you did.
.s1
"check" checks to see what your zdone state is.
.s1
The command will also show you how many countries want
an update.  There are 3 possible update policies which could
be set in this game.  The update policy is shown in the update command.
.s1
Your zdone state is cleared when an update occurs.
.s1
1. No demand updates.  If so, then this command is useless.
.s1
2. Updates occur upon demand, right after the last person sets
zdone.  The threshold at which a demand update occurs is configurable,
and may be seen in the update command.
.s1
3. Demand updates occur at specific times.  If enough countries
have set zdone at one of those times, then an update will occur.
The times may be seen in the update command.
.sp
.SA "update, Playing"
