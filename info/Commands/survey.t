.TH Command SURVEY
.NA survey "Show distributions in map form"
.LV Expert
.SY "survey <SELECTOR> <SECTS>"
The survey command shows the distribution
of some sector selector value in map form.
This is especially useful for commodities and resources.
.s1
If <SELECTOR> can have large values, like commodity selectors, then
survey shows hundreds, else tens of the value, as digits from 0 to 9.
Sectors which have a value less or equal zero show up as blank.
Sectors whose value exceeds 999 or 99 respectively show up as '$'.
.s1
Surveyed sectors will be highlighted.
.s1
If you supply conditions (see "info Syntax"), then sectors not
matching the conditions will not be surveyed, but have their current
designation displayed as in a map instead.
.s1
Sectors not owned by you will show up as blanks.
.s1
.SA "sect, map, Maps"
