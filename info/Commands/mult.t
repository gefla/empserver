.TH Command MULT
.NA mult "Set price multipliers of goods"
.LV Expert
.SY "mult <CNUM/CNAME> <NUMBER>"
The mult command allows a nation to set price multipliers
of the goods they are selling.
.sp
The <CNUM/CNAME> argument is the number or name of the nation
for which you want to reset the multiplier, and <NUMBER> is
the new multiplier (a floating-point number). If no
arguments are given, you are prompted for them. A carriage return
or incorrect nation at this prompt allows the changing
of all the price multipliers.
.sp
For example, if one nation is a \*Qnice\*U nation,
and another is particularly unfriendly,
the price multiplier for the nice one might be 0.75,
while the \*Qnasty\*U nation might have one around 5.
.s1
The price multiplier affects the price of a nation's goods
to other nations.
Final item price is determined by multiplying the price
of the item at the time of the sale by the individual price multiplier
for the nation buying the item.
You cannot have a price multiplier below 0.05.
.s1
Typical output looks like:
.EX mult
.NF
for which country? bar
bar (  1.000) : 3.00
.FI
.s1
.EX mult
.NF
for which country?
Editing nation price multipliers...
MGM (1.00) : 
hodedo (2.55) : 100
bar (1.00) : ^C
Aborted
.FI
.s1
The second editing session left unchanged all price multipliers except the
one for country \*Qhodedo\*U, which was set to 100.
.s1
.SA "buy, sell, Obsolete"
