.TH Command LOST
.NA lost "Report lost items"
.LV Basic
.SY "lost"
The lost command displays all lost items currently in the
database for your country.
.s1
All lost items are timestamped at the time they were lost.  If
you had a previously lost item in the database, and then build a
new item of the same type with the same id, or regain an item of the
same type with the same
id, it is removed from the database.  This is done because the new item
will now show up in the other dump commands, and is no longer lost.
.s1
Lost items are kept in the database for 48 hours (default) but may
be kept longer at the deity's discretion.  Also, issuing the lost
command does not remove items from the database.  That is where the
timestamp functionality becomes important.
.s1
While the lost command does not take any arguments, it does
take the standard syntax for selecting information.  
(see \*Qinfo Selector\*U).
.s1
A typical usage might be:
.EX lost ?timestamp>854954677
which would list data for all lost items timestamped after
854954677.  (Timestamps are kept in system seconds.)
.s1
A lost command lists all the lost items headed by:
.NF
Sun Feb  9 22:16:37 1997
DUMP LOST ITEMS 855544597
type id x y timestamp
.FI
The first line is the date.  The second line is the
"DUMP LOST ITEMS <timestamp>" where the <timestamp> field is the current
timestamp.  The third line is the columns which are output.
.s1
These columns represent:
.s1
.in \w'gtimestamp\0\0'u
.L type
The type of lost item.  They are as follows:
.NF
           0 - Sector
           1 - Ship
           2 - Plane
           3 - Land unit
           4 - Nuclear stockpile
.FI
.L id
The # of the ship, plane, land unit or nuke.  All of these items
are identified by id.  Sectors are identified by x and y.
.L x
The x coordinate of the lost item when it was lost.
.L y
The y coordinate of the lost item when it was lost.
.L timestamp
The timestamp of when the item was lost.
.in \\n(in
.s1
.SA "dump, ldump, sdump, ndump, pdump, Ships, Planes, LandUnits, Sectors, Clients"
