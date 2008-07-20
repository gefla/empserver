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
The command will also show you how many countries want an update.  The
number of ready countries required for a demand update to happen is
configurable, and may be seen in the \*Qshow updates\*U command.
.s1
There are three possible update policies which could be set in this
game:
.s1
1. No demand updates.  If so, then this command is useless.
.s1
2. Demand updates occur right after the last required country sets
zdone.
.s1
3. Demand updates occur according to the update schedule.  If enough
countries have set zdone by the scheduled time, then an update will
occur.  The update schedule may be seen in the \*Qshow updates\*U
command.
.s1
Your zdone state is cleared when an update occurs.
.s1
The \*Qshow updates\*U command shows update policy and schedule.
.s1
For deities, the command takes an additional parameter: the country to
impersonate.
.s1
.SA "show, Updates"
