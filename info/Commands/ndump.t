.TH Command NDUMP
.NA ndump "Dump raw nuke information"
.LV Expert
.SY "ndump <SECTS>"
The ndump command displays all information on
some or all of the sectors you occupy. 
Each nuke's information is printed on one very long line.
Fields are separated by a single space.
.s1
This command is designed to be used for input to an empire tool
such as \*Qve\*U.
.s1
In the syntax of the ndump command
<SECTS> is the area on which you wish information,
(see \*Qinfo Syntax\*U).
.s1
An ndump command lists all selected nuclear stockpiles headed by:
.NF
Sun Feb  9 22:16:37 1997
DUMP NUKES 855544597
id x y num type
.FI
The first line is the date.  The second line is the
"DUMP NUKES <timestamp>" where the <timestamp> field is the current
timestamp.  The third line is the columns which are output.
.s1
id - The id of the stockpile the nuke is in.
.s1
x and y - The x and y coordinates of the stockpile
.s1
num - The number of this type of nuke
.s1
type - This type of nuke (100kt, 5mt, etc.)
.s1
A typical usage might be:
.EX ndump #5
which would list data for all nukes in realm #5.
.s1
A ndump lists each of your sectors in the specified area.
The header line is a list of fields that correspond
to the order that ndump prints the nuke info.
.FI
.s1
.SA "nuke, Clients, Planes"
