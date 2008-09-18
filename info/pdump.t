.TH Command PDUMP
.NA pdump "Dump raw plane information"
.LV Expert
.SY "pdump <PLANES> [<fields>]"
The pdump command displays all information on
some or all of your planes.
pdump is a combination of plane and pstat.
Each plane's information is printed on one very long line.
Fields are separated by a single space.
.s1
This command is designed to be used for input to an empire tool
such as \*Qve\*U.
.s1
In the syntax of the pdump command
<PLANES> are the planes on which you wish information,
[<fields>] are the fields you wish to dump.
If no fields are specified, all fields are printed.
(see \*Qinfo Syntax\*U).
.s1
A pdump command lists all selected planes headed by:
.NF
Sun Feb  9 22:16:37 1997
DUMP PLANES 855544597
<fields>
.FI
The first line is the date.  The second line is the
"DUMP PLANES <timestamp>" where the <timestamp> field is the current
timestamp.  The third line is the columns which are output.
.s1
These columns represent the following fields which are also the
selection criteria for the command.
.s1
The following may be used as fields.  They must be entered EXACTLY as
shown here (no partial pattern matching is done.)
.NF
   type     type of plane
   x        x location
   y        y location
   wing     wing
   eff      efficiency
   mob      mobility
   tech     technology level
   att      attack value
   def      defense value
   acc      accuracy
   react    current reaction range
   range    maximum range
   load     load rating
   fuel     amount of fuel used in one flight
   hard     hardness
   ship     ship number the plane is on
   land     land unit number the plane is on
   laun     launched into orbit?
   orb      orbiting synchronously?
   nuke     nuke name (if armed)
   grd      groundburst? (if armed)
.FI
.s1
A typical usage might be:
.EX pdump #5
which would list data for all planes in realm #5.
.s1
A pdump lists each of your planes in the specified area.
The header line is a list of fields that correspond
to the order that pdump prints the plane info.
.s1
.SA "plane, pstat, Clients, Planes"
