.TH Command XDUMP
.NA xdump "Experimental dump of everything under the sun"
.LV Expert
.SY "xdump <STATE-TABLE> <ENTRIES>"
.SY "xdump chr <CONFIG-TABLE>"
.SY "xdump ver"
The xdump command displays information on game configuration and state
in machine readable tables.  It is intended for use by clients and
tools.
.s1
.L NOTE
This command is \fIexperimental\fP and thus only available if option
GUINEA_PIGS is enabled.  It may still change in incompatible ways.  It
is released in this early stage of development to show were we are
headed, and to solicit feedback.
.s1
The <STATE-TABLE> argument in
.SY "xdump <STATE-TABLE> <ENTRIES>"
is one of the following game state table names: sect (sectors), ship,
plane, land, nuke, news, treaty, trade, pow (power report), loan,
commodity (commodity market), lost (recently lost items).  Tables
power and nat do not work, yet.  Unique abbreviations of table names
are recognized.
.s1
The <ENTRIES> argument selects table entries to display, in common
Empire syntax for the kind of things in this table.
\" FIXME reference
For example, to dumps all sectors in realm #5, you can use:
.EX xdump sect 0,0
.NF
XDUMP sect 1103018538
owner xloc yloc des effic mobil terr terr1 terr2 terr3 xdist ydist avail work newdes min gold fert ocontent uran oldown off civil milit shell gun petrol iron dust bar food oil lcm hcm uw rad c_dist m_dist s_dist g_dist p_dist i_dist d_dist b_dist f_dist o_dist l_dist h_dist u_dist r_dist c_del m_del s_del g_del p_del i_del d_del b_del f_del o_del l_del h_del u_del r_del fallout access road rail dfense
1 0 0 5 100 127 0 0 0 0 8 0 6440 100 5 77 0 0 0 42 1 0 9900 1 0 0 0 0 1 0 50 0 0 0 0 0 0 0 0 0 0 0 1 0 50 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1078505792 0 0 99
dumped 1
.FI
This dumps all ships with mobility in fleet a:
.EX xdump ship a ?mobil>0
.s1
This dumps all loans:
.EX xdump loan *
.s1
For tables of things that have an owner (sect, ship, plane, land,
nuke), \*Qxdump\*U shows only things you own, unless you are a deity.
.s1
Table columns correspond to selectors used in Empire conditionals (see
\*Qinfo Selector\*U).  Naturally, \*Qxdump\*U shows columns
corresponding to selectors that only deities may access only to
deities.
.s1
.L "FUTURE DIRECTIONS"
We plan to extend the syntax to select columns to be shown.
.s1
The <CONFIG-TABLE> argument in
.SY "xdump chr <CONFIG-TABLE>"
is one of the following game configuration table names: sect (sector
characteristics), ship, plane, land, nuke, news, treaty, item,
infrastructure, product.  Table news does not work, yet.  Unique
abbreviations of table names are recognized.
.s1
.L "FUTURE DIRECTIONS"
We plan to extend the syntax to select rows and columns to be shown.
.s1
Finally,
.SY "xdump ver"
displays configuration parameters.
.s1
The output of \*Qxdump\*U is a table, which consists of two header
lines, the table body and a footer line.
.s1
The first header line is of the form \*QXDUMP <TABLE-NAME>
<TIMESTAMP>\*U.  It identifies the dump.
.s1
The second header line lists column names.  The name of a column is
the name of the corresponding selector.  If a name is followed by a
decimal number, it applies to that many columns, which form an array.
.s1
The body consists of records; one record per line.
.s1
A record consists of fields separated by a space.  All records have
the same number of fields, which matches the header.
.s1
A field is either in integer, floating-point number or string format.
In no case does it contain space or newline.  All fields in the same
column have the same format.
.s1
Integer fields are in decimal; they can be parsed by scanf %d.
Conversely, if that succeeds, it's an integer field.
.s1
Floating-point fields can be parsed by scanf %g (or equivalents %e,
%f).  Conversely, if that succeeds, it's a floating-point field.
.s1
String fields are in C syntax.  Funny characters including space,
newline and doublequote are shown as octal escape sequences.  Example:
"gold\\040mine".  If a field starts with a double-quote, it's a string
field.  Here's a scanf format string: "\\"%[^\\"]\\"".  You have to
postprocess the value to translate escapes.
.s1
.L "FUTURE DIRECTIONS"
We might decide to use simple escape sequences as well, but never \\".
.s1
Some columns can contain null strings.  A null string is the three
letters nil.  If a field contains nil, it's a string field.
.s1
If you know all the fields and their format, and none of them can be a
null string, you can parse a complete record with a single scanf.  The
null string exception is unfortunate.
.s1
If you don't know field formats, you can still parse field by field.
.s1
.L "FUTURE DIRECTIONS"
We plan to add meta-dumps describing the dumps.  Without such
meta-data, tools have to know the server's internal coding of various
.L BUGS
\*Qxdump\*U discloses all units, regardless of player's tech level.
\*Qxdump\*U fails to suppress empty rows in some tables.
.s1
.SA "dump, ldump, ndump, pdump, sdump, lost, Clients, Communication, LandUnits, Planes, Sectors, Ships"
