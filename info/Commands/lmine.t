.TH Command LMINE
.NA lmine "Mine a sector with an engineering unit"
.LV Expert
.SY "lmine <UNIT> <NUMBER>"
This command causes a unit with the engineering ability
to lay mines in a sector. (see \*Qinfo Unit-types\*U).  If you ask to
lay more mines than the unit is carrying, then it will attempt to keep
resupplying itself until all of the mines have been laid.  It costs
1 mobility point per mine laid, so an engineer with 20 mobility can
lay a maximum of 20 mines.  The 'strength' command will list how many
mines you have put in each sector.
.s1
See \*Qinfo Hitchance\*U for the chance of something hitting a land mine.
.s1
If a defender in land combat has land-mines in the sector, he
receives a bonus to his defense. See info \*Qattack\*U for more information.
If the attacker has engineering units with him, the bonus is less.
.s1
In the command syntax
<UNIT> is the number of one of your units and <NUMBER> is
the number of mines you wish to lay.
That number of shells, (assuming you've got them),
will be dropped in the sector that unit occupies,
magically becoming mines.
.s1
You need not fear mines in sectors that you are the old owner of. This
is normally all your sectors. When you capture a mined sector, however,
you will need to worry about those mines until you become the old owner
of that sector. Once you are the old owner of a sector (i.e when you
get the message "Sector x,y is now fully yours."), you have the mines located,
and will no longer hit them.
.s1
If you are in a captured sector, and the original owner attacks, you will not
get the mine bonus (as he knows where all the mines are)
.s1
Mines can only be removed safely by a unit with the engineering ability.
(Other units can remove them by hitting them, but...)
Engineering units moving through a sector with unlocated mines will
attempt to remove them.  The number of times each engineer sweeps for
mines is equal to 2 times the number of mines it can carry.  The
chance of the engineer finding a mine on each sweep is equal to 50%
times its 'att' value (see show land stats).  If an engineer sweeps
more mines than it can carry, then it will drop the rest as shells
into the sector.
.s1
.SA "march, strength, move, explore, transport, Hitchance, Damage, Attack, LandUnits"
