.TH Command XDUMP
.NA xdump "Extended dump of everything under the sun"
.LV Expert
.SY "xdump <TYPE|NUMBER> <RECORDS>"
.SY "xdump meta <TYPE|NUMBER>"
The xdump command displays information on game configuration and state
in machine readable format.  It is intended for use by clients and
tools.  This info page gives an overview; see doc/xdump in the source
distribution for complete documentation.
.s1
Game configuration and state consists of a a number of tables or
files.  xdump shows only things that are \*(fIcertainly\*(fP visible to
you: sectors you own, ship types known, etc.
.s1
The output of xdump is a table consisting of a header line, the table
body and a footer line.
.s1
The header line is of the form \*QXDUMP <TYPE> <TIMESTAMP>\*U.  <TYPE>
identifies the dump.  The timestamp permits incremental dumps (see
below).
.s1
The body consists of records; one record per line.  A record consists
of fields separated by a space.  All records have the same number of
fields.  Fields match records in the table's meta-table (see below).
.s1
A field is either in integer, floating-point number or string format.
In no case does it contain space or newline.  All fields in the same
column have the same format.
.s1
Integer fields are in decimal; they can be parsed by scanf %d.
Conversely, if that succeeds and consumes the complete field, it's an
integer field.
.s1
Floating-point fields can be parsed by scanf %g (or equivalents %e,
%f).  Conversely, if that succeeds, it's a floating-point field.
.s1
String fields are in C syntax.  Funny characters including space,
newline and double-quote are shown as octal escape sequences.  Example:
"gold\e040mine".  If a field starts with a double-quote, it's a string
field.
.s1
.L "FUTURE DIRECTIONS:"
We might decide to support more C escape sequences, but never \\\(dq.
.s1
Some fields can contain null strings.  A null string is the three
letters \*Qnil\*U.  If a field contains nil, it's a string field.
.s1
Each table has a meta-table, which defines format and meaning of its
fields.  To show the meta-table for <TYPE>, use
.EX xdump meta <TYPE>
.s1
All meta-tables share the same meta-meta-table, which is its own
meta-table.  The command to show it is
.EX xdump meta meta
.s1
Fields correspond to selectors used in Empire conditionals (see
\*Qinfo Selector\*U), and they use the same names.  Naturally, xdump
shows fields corresponding to deity-only selectors only to deities.
.s1
The command to display table <TYPE> is
.EX xdump <TYPE> <RECORDS>
.s1
Argument <RECORDS> selects which records to show, in common Empire
syntax for the kind of things in that table.  \*Q*\*U always shows the
complete table.
.s1
To dump sector -22,2, for example, you can use:
.EX xdump sect -22,2
.s1
This dumps all ships with mobility in fleet a:
.EX xdump ship a ?mobil>0
.s1
This dumps all sectors changes after timestamp 1141068862:
.EX xdump sect * ?timestamp>1141068862
.s1
Use this with a value one less than the timestamp in the xdump header
to get everything that changed since that xdump (incremental dump).
.s1
.L "FUTURE DIRECTIONS:"
We plan to extend the syntax to select fields to be shown.
.s1
The table \*Qtable\*U is the table of (non-meta-)tables.  It contains
table names and numbers.  xdump accepts table numbers as well as
names.
.s1
There's a special pseudo-table \*Qver\*U, which collects configuration
parameters in a single record.  It is not listed in \*Qtable\*U, it
doesn't have a table number, but it does have a meta-table.
.s1
.L BUGS:
Still missing are game state files power, map, bmap; the list of
commands; foreign levels, contact
information, mortal access to sector selector
mines, ship selectors xbuilt, ybuilt, trade selectors xloc, yloc,
commodity selectors xbuy, ybuy, xsell, ysell.  Some information is not
yet available in human-readable form: the lost file, sect-chr selector
nav, ship and land plague status (deity only), plane theta (governs
satellite movement), trade selectors xloc, yloc, commodity selectors
xbuy, ybuy.
Incremental dump doesn't work after importing news with the empdump
utility.
.s1
.L "FUTURE DIRECTIONS:"
xdump is still fairly new, and experience with it may lead to changes.
Client writers should be prepared for that.
.s1
.SA "dump, ldump, ndump, pdump, sdump, lost, show, version, Clients, Communication, LandUnits, Planes, Sectors, Ships"
