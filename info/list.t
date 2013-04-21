.TH Command LIST
.NA list "List commands which are currently legal"
.LV Expert
.SY list
The list command gets you a list of currently legal commands
with their associated BTU costs,
(see \*Qinfo time\*U for description of BTU's).
This should not be confused with \*Qinfo {commands}\*U
which generates a brief summary of each command.
.s1
.EX list
.NF
		Current EMPIRE Command List
		------- ------ ------- ----
Initial number is cost in B.T.U. units.
Next 2 chars (if present) are:
$ - must be non-broke	c -- must have capital
Args in [brackets] are optional.
All-caps args in <angle brackets> have the following meanings:
  <NUM> :: a number in unspecified units
  <COMM> :: a commodity such as `food', `guns', etc
  <TYPE> :: an item type such as `ship', `plane', etc
 0    accept  [as-COUNTRY]
 0    add <NAT> <NAME> <REP> <STATUS>
 0    announce
 3 $c anti <SECTS>
     etc.
.FI
.SA "Syntax, Time, Bugs, Playing"
