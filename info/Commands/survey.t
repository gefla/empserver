.TH Command SURVEY
.NA survey "Show distributions in map form"
.LV Expert
.SY "survey <ITEM> <SECTS> ?cond<op>cond"
The survey command shows the distribution
of a commodity or resource in map form.
Sectors which are owned by you,
but do not match the conditional
have their current designation displayed much as in a map.
Sectors matching the conditional are shown in one of several ways.
.s1
Sectors which have none (or a level of zero) of the item being
surveyed show up as a blank.
.s1
If <ITEM> is a commodity,
then sectors having the commodity present
will be shown as a digit from 0 to 9,
the digit representing the number
of hundreds of the commodity present in the sector.
Sectors with more than 999 of a given commodity will be seen as a '$'.
.s1
If <ITEM> is a sector resource,
then the 0 to 9 digits will represent
the number of tens of the resource present in the sector.
Sectors with more than 99 of a given resource will show up as a $.
.s1
If your terminal can support standout mode,
then sectors selected will be highlighted.
.s1
Sectors not owned by you will show up as blanks.
.s1
NOTE: the syntax of this command is somewhat bizarre,
as the conditional \*Qcond<op>cond\*U
is for all practical purposes NOT optional
if the survey is to display any values.
.s1
.SA "sect, map, Maps"
