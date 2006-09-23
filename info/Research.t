.TH Concept Research
.NA Research "How Research level is calculated each update"
.LV Expert
.sp
.s1
If the RES_POP option is in use, 
the research level of a country determines the maximum number of civilians
and uw's
that can live in a sector. For ordinary sectors, the maximum is:
.s1
.ti 3
min(1000, 400 + 600 * ((research*4 + 50)/(research*3 + 200)))
.s1
So, for example, at 0 research, after an update, the max you
could have in a sector would be 550 civs and 550 uws.
Note that in the case of captured sectors,
the research level of the current owner governs, even if the old
owner of the sector had a higher/lower research.
.s1
If, after population growth, there are more civs than the limit, the
excess are lost.  Uw's work the same way.
.s1
The research level also affects the likelihood of plague outbreaks.
(Unless the NO_PLAGUE option is used, of course)
.s1
The \*Qnation\*U command will display,
among other things,
your current research level.
.s1
The procedure determining how much your research level goes up every
update is exactly the same for research as it is for technology
(see "info Technology") except for the fact that "research log base"
is always 2.00 and "easy research" is always 0.75.  But besides this,
research bleeds, decays, and leaks in exactly the same way as technology.
.s1
.SA "Innards, nation, Products, Technology, Nations, Producing, Updates"
