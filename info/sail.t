.TH Command SAIL
.NA sail "Give sailing orders to a ship"
.LV Expert
.SY "sail <SHIP/FLEET> [<PATH>|q[uery]|-]"
You use the \*Qsail\*U command to give a sailing path to ships.
At each update, ships with a sailing path \*Qauto-navigate\*U along
the path that was set for them.  Telegrams notify you of arrivals
or problems encountered while attempting to follow the path.
.s1
Radar operates continuously as the ship moves along the path, constantly
adding sector information to the known world map.
.s1
Ship movement is performed before mobility is increased.
This insures that after the update, ships with a sailing path will still
have some mobility so that you can always manually navigate
to some extent.
.s1
Note that ships sailing can encounter exactly the same interdiction as
ships navigating.
.s1
.EX sail <SHIP/FLEET> q
.s1
\*Qsail\*U, with the \*Qquery\*U parameter, shows the current sailing path.
The report indicates:
.NF
     shp#          ship number
     ship type     type of ship (cargo ship, destroyer, etc)
     x,y           ship's current position
     mobil         ship's current mobility
     mobquota      mobility available per update to sail
     follows       ship that is being followed
     path          the current sailing path

shp#     ship type       x,y    mobil mobquota follows path
   1 cargo ship        -27,19    44      32          1 jjjuujj
  12 cargo ship        -27,19    40      32          1
  34 fishing boat       15,32    58      32          3 ggbbgyyg
  58 cargo ship         18,34    62       0          4
4 ship(s)
.FI
.s1
.EX sail <SHIP/FLEET> -
.s1
\*Qsail\*U, with the \*Q-\*U parameter, causes the sailing path
for the ships specified to be removed.
.s1
The PATH is a normal empire path specification. You may *not* give a
destination sector on the command line, or at any time while giving
the route.  Use the \*Qorder\*U command if you wish this functionality.
.s1
When getting a path interactively, empire will show you the information
you have (from your bmap) concerning the current area, to help you plot
your course.
.s1
.SA "follow, unsail, mquota, Ships"
