.TH Command DISTRIBUTE
.NA distribute "Establish distribution destination sector"
.LV Basic
.SY "distribute <SECTS> <DISTSECT|.|h>"
The distribute command is used to specify the way that goods will
travel during an update.
Each sector is allowed to have one and only one
\*Qdistribution sector\*U (a warehouse or road, typically).
Subsequent distribute commands
will replace the existing distribution sector.
.s1
For example,
.EX distribute 2,0 8,4
.NF
highway at 2,0 delivers to 8,4. 
Distribution sector? 7,-1
highway 2,0 new path distributes to 7,-1 (cost 0.200)
        Current best path is 'gguh'
.FI
This command changed the distribution path for sector 2,0 from
a warehouse at 8,4 to a warehouse at 7,-1. The cost is the mobility cost
to move 1 normal-weight unit from the sector to the warehouse.
.s1
DISTSECT is a normal empire sector specification. During the update,
empire will determine the cheapest existing path to the distribution
sector, and use that. (Note that the mobility cost for the path used
during the update could
be different than the cost reported to you when you use the distribute command,
if you have re-designated sectors, or sectors were captured, etc)
Again, it will use the cheapest path available AT THE TIME OF THE UPDATE.
.s1
You may also use '.' or 'h' to specify no distribution sector.
.s1
.EX dist 2,0 .
.NF
Distribution to and from 2,0 halted.
.FI
.s1
The command will warn you if there is no existing path completely
owned by you, and also if the path does not end at a warehouse.
It is not required that the distribution path end at a warehouse,
but it is required that you own all the sectors that the goods
will travel along.
.s1
.L "Distribution & the Update"
.s1
One of the last things that takes place during the update, after all growth,
production, delivery, etc, distribution takes place. (The only thing that
really happens after that is mobility increases) Each sector may have
product
thresholds and a distribution sector. A threshold is a target level for a
particular commodity, like food. So, if the food threshold for a sector is
100, it will try to maintain 100 food there. If it has more than 100 food,
it will export it to its distribution point. If it has less than 100, it
will try to import food from the distribution point to reach 100.
.s1
During the distribution phase of the update, all sectors (going left to
right, top to bottom) first export excess
commodities, and then (again left to right, top to bottom)
import needed commodities.
.s1
In order to either send things to or get things from a distribution sector,
you must own the distribution sector, and there must be a path of owned
sectors to it. In order to export something from a sector, you must have
military control of the sector. In order to import something from a
distribution sector, you must have military control of the distribution
sector. (Military control is defined as having mil in the sector equal
to at least 1/10th the number of unconverted civilians there. Units count
as if they were straight mil, i.e. a 100% unit that contained 50 mil would
count as 50 mil for the purposes of control)
.s1
Mobility is used from the sector when sending \*Qto\*U the distribution sector.
When getting something \*Qfrom\*U the distribution sector, mobility is paid
by the distribution sector.  See \*Qinfo Mobility\*U for an
explanation of mobility costs.
.s1
.L Amount
.s1
The amount is determined by the threshold, as explained above.
Use the \*Qthreshold\*U command to specify what goods
will flow to and from the warehouse.
.s1
.L "Finding sectors with not dist point set"
.s1
If you want to find sectors with no distribution point set, use the
xdist and ydist selectors:
.s1
.EX level # ?xdist=xloc&ydist=yloc
.s1
This finds all sectors that have themselves as a distribution point.
.s1
.L "Distributing civilians and military"
.s1
When distributing civilians or military,
no distribution point will ship out its last civilian or
the last military if there are no civilians,
regardless of thresholds set in sectors using it as a
distribution point.
.s1
.SA "deliver, threshold, level, Distribution"
