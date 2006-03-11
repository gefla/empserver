.TH Command DUMP
.NA dump "Dump raw sector information"
.LV Expert
.SY "dump <SECTS> [<fields>]"
The dump command displays all information on
some or all of the sectors you occupy. 
dump is a combination of census, commodity, cutoff and level.
Each sector's information is printed on one very long line.
Fields are separated by a single space.
If no fields are specified, all fields are printed.
.s1
This command is designed to be used for input to an empire tool
such as \*Qve\*U.
.s1
In the syntax of the dump command
<SECTS> is the area on which you wish information,
[<fields>] are the fields you wish to dump.
If no fields are specified, all fields are printed.
(see \*Qinfo Syntax\*U).
.s1
A dump command lists all selected sectors headed by:
.NF
Sun Feb  9 22:16:37 1997
DUMP SECTOR 855544597
<fields>
.FI
The first line is the date.  The second line is the
"DUMP SECTOR <timestamp>" where the <timestamp> field is the current
timestamp.  The third line is the columns which are output.
.s1
The following may be used as fields.  They must be entered EXACTLY as
shown here (no partial pattern matching is done.)
.s1
.NF
   des
   sdes
   eff
   mob
   *
   off
   min
   gold
   fert
   ocontent
   uran
   work
   avail
   terr
   civ
   mil
   uw
   food
   shell
   gun
   pet
   iron
   dust
   bar
   oil
   lcm
   hcm
   rad
   u_del
   f_del
   s_del
   g_del
   p_del
   i_del
   b_del
   o_del
   l_del
   h_del
   r_del
   u_cut
   f_cut
   s_cut
   g_cut
   p_cut
   i_cut
   d_cut
   b_cut
   o_cut
   l_cut
   h_cut
   r_cut
   dist_x
   dist_y
   c_dist
   m_dist
   u_dist
   f_dist
   s_dist
   g_dist
   p_dist
   i_dist
   d_dist
   b_dist
   o_dist
   l_dist
   h_dist
   r_dist
   road
   rail
   defense
   fallout
   coast
   c_del
   m_del
   c_cut
   m_cut
   terr1
   terr2
   terr3
.FI
.s1
A typical usage might be:
.EX dump #5
which would list data for all sectors in realm #5.
.s1
A dump lists each of your sectors in the specified area.
The header line is a list of fields that correspond
to the order that dump prints the sector info.
.s1
.SA "census, commodity, cutoff, level, xdump, Clients, Sectors"
