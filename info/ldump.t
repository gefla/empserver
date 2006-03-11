.TH Command LDUMP
.NA ldump "Dump raw land unit information"
.LV Expert
.SY "ldump <UNITS> [<fields>]"
The ldump command displays all information on
some or all of your land units. 
ldump is a combination of land, lcargo, and lstat.
Each ship's information is printed on one very long line.
Fields are separated by a single space.
.s1
This command is designed to be used for input to an empire tool
such as \*Qve\*U.
.s1
In the syntax of the ldump command
<UNITS> are the land units on which you wish information,
[<fields>] are the fields you wish to dump.
If no fields are specified, all fields are printed.
(see \*Qinfo Syntax\*U).
.s1
An ldump command lists all selected land units headed by:
.NF
Sun Feb  9 22:16:37 1997
DUMP LAND UNITS 855544597
<fields>
.FI
The first line is the date.  The second line is the
"DUMP LAND UNITS <timestamp>" where the <timestamp> field is the current
timestamp.  The third line is the columns which are output.
.s1
These columns represent the following fields which are also the
selection criteria for the command.
.s1
The following may be used as fields.  They must be entered EXACTLY as
shown here (no partial pattern matching is done.)
.NF
   type    type of land unit
   x       x location
   y       y location
   army    army
   eff     efficiency
   mil     # of mil loaded
   fort    fortification level
   mob     mobility
   food    # of food loaded
   fuel    # of fuel loaded
   tech    technology of unit
   retr    retreat %age of unit
   react   current reaction radius 
   xl      # of x-light planes loaded onto unit
   nland   # of land units loaded onto unit
   land    land unit # this unit is loaded on
   ship    ship # this unit is loaded on
   shell   # of shells loaded
   gun     # of guns loaded
   petrol  # of petrol loaded
   iron    # of iron loaded
   dust    # of dust loaded
   bar     # of bars loaded
   oil     # of oil loaded
   lcm     # of lcms loaded
   hcm     # of hcms loaded
   rad     # of rads loaded
   att     attack value of unit
   def     defense value of unit
   vul     vulnerability of unit
   spd     speed of unit
   vis     visibility of unit
   spy     spy ability of unit
   radius  maximum reaction radius
   frg     firing range of unit
   acc     accuracy of unit
   dam     damage unit does when firing
   amm     amount of ammo used when firing
   aaf     anti-aircraft fire provided
.FI
.s1
A typical usage might be:
.EX ldump #5
which would list data for all land units in realm #5.
.s1
A ldump lists each of your land units in the specified area.
The header line is a list of fields that correspond
to the order that ldump prints the land unit info.
.s1
.SA "land, lcargo, lstat, xdump, Clients, LandUnits"
