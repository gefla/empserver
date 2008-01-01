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
Initial number is cost in BTU's.
The next 2 character (if present)
Next 2 chars (if present) are:
$ - must be non-broke c -- must have capital
Text enclosed in {braces} is comment rather than part of the command.
Args in [brackets] are optional, but can only appear on the command line.
Args in <angle brackets> have the following meanings:
  <CNUM/CNAME> :: a country number or name
  <ITEM> :: commodity such as `ore', `guns', etc
  <LOAN> :: loan number
  <SECT> :: sector coordinates in the form:   x,y
  <SECTS> :: sector(s) in the form:   lox:hix,loy:hiy/d ?cond&cond&...  
  <SHIP> :: one ship number
  <FLEET> :: fleet designation, or area
2    announce                          2 arm <PLANE> <WARHEAD> <NUMBER>
2 $c assault <SECT> {from ship}        2 attack <SECT> <move-in> {from sect}
     etc.
.FI
.SA "Syntax, Time, Bugs, Playing"
