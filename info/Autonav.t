.TH Concept Autonavigation
.NA Autonav "Giving ships automatic sailing orders"
.LV Expert
In a very short while after acquiring ships the effort of moving ships after
every update gets very boring.  And the more ships you have, the more work
it is.  Hence the concept of autonavigation.  Like any self-respecting
dictator, you don't personally drive your ships around (except when you want
to), instead you command you ship captains to take their vessels to a
specified destination.  This is done via the \*Qorder\*U command.
.s1
Each ship can be ordered to proceed to a destination.  During each update,
the ship will try and use ALL the mobility it has and use the shortest
possible path.  It will also avoid any known mines (sectors marked
with an 'X' or 'x' on your bmap).  Note that this movement is all 
done before mobility is accumulated, so that after the update is
complete the ship will have a full updates worth of mobility
in case you need to move it by hand.  Should a ship be fired upon
by forts or strike a mine they will stop moving!  This will prevent
your ship from foolishly getting sunk in enemy waters.  The
\*Qorder\*U command covers many special cases.

.s1
The autonavigation capability is dependent upon the per country map database
accessed via the \*Qbmap\*U command.  If you order a vessel to a destination
that causes it to cross a previously unexplored area, the ship will attempt
to cross that area as if it were water.  One of the features of the bmap
functionality is that when a ship is navigated, it automatically uses its
radar to see the local sectors (much like real life).  This sector information
is added to the per country map database.  Thus upon running into a previously
unknown obstruction, that information is added to the map database and on the
next update, a new path to the destination will be calculated which avoids
the obstruction.  This process applied iteratively means that a ships will
eventually find its way around any obstruction, provided that a path exists.
In reality, this is a limit to this process.  In order to limit the processing
that is done to find a path to the destination, the internal path string is
limited to 28 characters.
.s1
The other capability of the autonav function is autotrading.  By specifying
two destinations and two commodities, the ship will move back and forth between
the two destination loading and unloading the appropriate commodities.
Specific examples can be found in the \*Qorder\*U info file.
.s1
Autonavigation also supports automatic resource production at sea.
See info \*QAutofish\*U for details.
.s1
The same rules for navigating, loading, and unloading apply for
Autonavigation as they do when you do these things by hand, namely
that you may only use harbours owned by nations which consider you to
be a friendly trading partner (see info relations).
.s1
.SA "navigate, order, Ships, Updates, Autofish"
