.TH Concept Plague
.NA Plague "The Plague"
.LV Expert
.s1
Plague is used in Empire to symbolize all forms of disease and is
the only health hazard associated with the Empire game,
(ignoring it's effects on the players).
The characteristics of plague are, therefore,
a compromise of disease features (or \*Qbugs\*U).
.s1
Note that if the NO_PLAGUE option is in use, a kindly deity
has wiped out the plague for you. If not, worry about it.
.s1
All of the timelines below are based on a game with 32 ETU updates.
.s1
The Empire Plague takes between 96 and 189 time units to
run its course on land,
(i.e., 2 to 4 days in any particular sector/ship/land unit
if an Empire time unit is 1/2 and hour.)
In so doing the plague goes through three stages:
.s1
.in +3
.ti -3
Stage I -- The gestation or incubation phase
in which no symptoms appear.
Sectors or ships that are in this stage of the plague
are indistinguishable from healthy sectors or ships.
This stage lasts 32 to 63 time units,
(less than a day and a half).
.s1
.ti -3
Stage II -- The infectious phase in which the symptoms first appear --
orange blotches on the face and hands,
sometimes accompanied by grey stripes on the genitalia,
itching of the liver and spleen,
an uncontrollable fear of ripe tomatoes, etc.
During this phase the plague is extremely communicable;
for example, a simple delivery from an infectious sector
will infect the destination sector.
.s1
.ti -3
Stage III -- The terminal phase in which people die,
often in the act of fleeing from (real or imagined) tomatoes.
The number of people that die is roughly inversely proportional
to the research level of the country (plus 100).
.in -3
.s1
All three phases are of roughly equal average duration,
(about 48 time periods or 1 day).
.s1
Plague usually arises in countries with high technological development
and comparatively little medical research.
It arises specifically in sectors with high population density
and low efficiency.
.s1
Recently archaeologists have uncovered a treatise
by a Dr.\ M.\ Welby entitled
\*QDemographic considerations and the Empire Plague\*U.
In part it states:
.s1
.in +0.8i
.ev 2
.ll 5.5i
It appeared that our original hypothesis
based on the high percentage of Lumbagan Legionnaires
among the afflicted had led us down a blind alley
and that if we were to solve this complex puzzle
before the end of the series in the spring
\*[possibly a reference to the series of tests
leading to the mysterious \*QNeilson Rating\*U\*]
we would have to turn to other disciplines for help.
It was only by the merest lucky coincidence
that as I drove home one evening ...
\*[here Dr. Welby relates an amusing anecdote
about a Brownie Scout and the director of a film
entitled \*QClose Encounters of the Third Grade\*U\*] ...
leading us to the following amazing formulation
of the relationship between medical research,
technology, population, standard of living and the Empire Plague:
.ll 6.5i
.ev
.in
.s1
.NF +0.4i
likelihood   civ + mil + uw   t_level + (iron + oil + rad * 2) / 10 + 100
    of     = -------------- * -------------------------------------------
  plague           999               r_level + effic + mobil + 100
.FI
.s1
Later formulations of this relationship show great similarity
to his statement (see \*Qinfo Innards\*U).
.s1
Many researchers have commented on the surprising lack of correlation
between food supply and plague outbreak.
.s1
Fortunately, the plague is only infectious while the sector is
actively battling the plague (Stage II).  The most effective method
for curing plague has been to isolate the sector by stopping all
distribution, deliveries, movement, etc.  originating from the sector
once an outbreak has been spotted.
.s1
This includes loading and unloading of ships and land units (since
they can, of course, carry plague), allowing explorations out of the
sector, moving commodities out of the sector, distribution or
deliveries from the sector, etc.  Basically, any people or commodities
that originate from a sector actively battling plague will cause the
destination sector (or ship or land unit) to become infected.
.s1
Once an outbreak has been spotted, the plague takes on an infectious
state at the next update.  There is very little time to isolate the
sector and limit deaths caused by plague.
.s1
.SA "Innards, Research, Sector-types, Technology, Populace, Update"
