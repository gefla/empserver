.TH Command CEDE
.NA cede "Give a ship or sector to a friend"
.LV Expert
.SY "cede <SECTORS|SHIPS> <COUNTRY> [se|sh]"
This command gives a sector or sectors to the country of your choice, or 
a ship or ships.
A sector to be ceded must have mobility, and must be adjacent to a
sector with mobility owned by the country you are ceding it to, \*QOR\*U
contain a ship owned by the country you are ceding it to, \*QOR\*U a
sector owned by the recipient that contains a land unit with mobility.
.s1
A ship to be ceded must either be in a sector owned by the recipient, or
in a sector containing a ship owned by the recipient.
.s1
You may only cede to nations which consider you a friendly trading partner.
.s1
All contents of the sector become the property of the new owner,
such as commodities, planes, land units, etc. Ships, however, do
NOT change ownership.
.s1
All contents of a ceded ship become the property of the new owner, including
planes or land units. Distribution & delivery info of ceded sectors are wiped.
.s1
The ceded sector has its mobility set to zero. If the owner of the sector
was not the old owner when he ceded it, the old owner is not changed.
The mobility of a ceded ship is not changed.
.s1
If it is ambiguous whether you wish to cede a sector or a ship, then
you can specify a third argument of either sh or se meaning ship and
sector respectively.
.s1
For example:
.EX cede 2,2 Rongovia
.NF
Sat Jan 16 22:33:11 1993
  sect eff
  2,2 100% ceded
    1 sector
.FI
.s1
The recipient gets a telegram telling him of the gift:
.NF
> BULLETIN from POGO, (#0)  dated Sat Jan 16 22:33:11 1993
-11,-3 ceded to you by Groonland
.FI
.s1
.SA "Sectors"
