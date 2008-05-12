.TH Command SDUMP
.NA sdump "Dump raw ship information"
.LV Expert
.SY "sdump <SHIPS> [<fields>]"
The sdump command displays all information on
some or all of your ships. 
sdump is a combination of ship, cargo, and sstat.
Each ship's information is printed on one very long line.
Fields are separated by a single space.
.s1
This command is designed to be used for input to an empire tool
such as \*Qve\*U.
.s1
In the syntax of the sdump command
<SHIPS> are the ships on which you wish information,
[<fields>] are the fields you wish to dump.
If no fields are specified, all fields are printed.
(see \*Qinfo Syntax\*U).
.s1
An sdump command lists all selected ships headed by:
.NF
Sun Feb  9 22:16:37 1997
DUMP SHIPS 855544597
<fields>
.FI
The first line is the date.  The second line is the
"DUMP SHIPS <timestamp>" where the <timestamp> field is the current
timestamp.  The third line is the columns which are output.
.s1
These columns represent the following fields which are also the
selection criteria for the command.
.s1
The following may be used as fields.  They must be entered EXACTLY as
shown here (no partial pattern matching is done.)
.NF
   type    type of ship
   x       x location
   y       y location
   flt     fleet
   eff     efficiency
   civ     # of civilians on board
   mil     # of mil on board
   uw      # of uw on board
   food    # of food on board
   pln     # of planes on board
   he      # of helicopters on board
   xl      # of x-light planes on board
   land    # of land units on board
   mob     mobility
   fuel    obsolete, retained for backwards compatibility, always zero
   tech    technology of ship
   shell   # of shells on board
   gun     # of guns on board
   petrol  # of petrol on board
   iron    # of iron on board
   dust    # of dust on board
   bar     # of bars on board
   oil     # of oil on board
   lcm     # of lcms on board
   hcm     # of hcms on board
   rad     # or rads on board
   def     defense of ship
   spd     speed of ship
   vis     visibility of ship
   rng     firing range of ship
   fir     max # of guns fired by each volley
   name    ship name
   origx   x origin where trade ship was built
   origy   y origin where trade ship was built
.FI
.s1
A typical usage might be:
.EX sdump #5
which would list data for all ships in realm #5.
.s1
A sdump lists each of your ships in the specified area.
The header line is a list of fields that correspond
to the order that sdump prints the ship info.
.s1
.SA "ship, cargo, sstat, xdump, Clients, Ships"
