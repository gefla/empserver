.TH Command DESIGNATE
.NA designate "Specify sector utilization"
.LV Basic
.SY "designate <SECTS> <DESIGNATION>"
In Empire all sectors have a \*Qdesignation\*U.
The \*Qdesignation\*U represents the principal industry or activity
taking place in that area.
When a sector is designated a harbor, for example,
the civilians in the sector start building shipyards and docks,
(with a little help from any military present).
.s1
If the redesignated sector is already declared to be something else,
and is efficient, then the workers will have to tear down the existing
construction and then build the new sector.
The workers dismantle existing sectors four times faster than they
can build.
.s1
As work progresses the efficiency will climb toward a maximum of 100%.
.s1
If you don't specify the <DESIGNATION> argument
in the command line as in:
.EX designate 1:3,2:5
The program will ask for each sector specified
what you want the new sector designation to be.
.s1
However, if you wish to designate one or several
sectors to be one particular thing and don't want to
be asked on each one you can type something like:
.EX designate -6:6,-3:3 ?type=f +
This will designate all fortresses that you own within
three sectors of your capital to be highways.
.s1
Note that you cannot re-designate bridge spans.
.s1
Normally, each point of efficiency added costs you $1. Some sectors
may cost more, and some may also require the expenditure of lcms and/or
hcms from the sector. In addition, some sectors may cost money merely
to designate. (see the \*Qshow\*U command and \*Qinfo show\*U for details)
.s1
.SA "Sector-types, Sectors"
