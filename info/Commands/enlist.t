.TH Command ENLIST
.NA enlist "Turn civilians into military"
.LV Basic
.SY "enlist <SECTS> <NUMBER>"
The enlist command converts civilians into military
in the sectors specified.
Military \*Qcreated\*U by the enlist command are actually reserves
called up from your country's military reserve.
If your country has no military reserves
(created by \*Qdemobilize\*U)
then no civilians will be converted and no military will be generated.
.s1
After the program has collected any necessary information,
it will list each sector where civilians answer the call
and the number who showed up followed by
the total number of military resulting (in parentheses).
.s1
If <NUMBER> is a positive number that many draftees will be sought in
each sector regardless of any military already in the sector.
.s1
If <NUMBER> is a negative number of the form -\fBxxx\fP,
then an enlistment quota is established
and the draft board will attempt to enlist enough in each sector
to bring the total number of military up to \fBxxx\fP in each sector.
.s1
Although the program tries to draft the specified number of
civilians in each sector only one-half of the civilians
in a sector can be called upon at any one time.
.s1
Civilians in conquered sectors (the conquered populace)
will not serve in your armed forces.
.s1
.EX enl -3:3,-1:2 10
.NF
Number to enlist (max 397) : 22
 22 enlisted in -1,-1 (30)
 13 enlisted in 1,-1 (13)
 22 enlisted in 3,-1 (30)
 17 enlisted in -2,0 (17)
 22 enlisted in 0,0 (36)
 22 enlisted in -1,1 (34)
 22 enlisted in 1,1 (34)
 18 enlisted in 0,2 (18)
 22 enlisted in 2,2 (45)
Total new enlistment : 180
Paperwork at recruiting stations ... 3.6
.FI
Note that there is a cost of 0.02 BTU's per draftee in addition to the
basic cost of the command and enlistees use up some mobility in the
enlistment process (see \*Qinfo Mobility\*U for the exact formula).
.s1
.SA "demobilize, Mobility, Populace"
