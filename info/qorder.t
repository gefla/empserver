.TH Command QORDER
.NA qorder "Query the auto-navigation orders of a ship"
.LV Expert
.SY "qorder <SHIP/FLEET>"
.s1
\*Qqorder\*U shows current commodity levels on a ship.
Ships with no orders are not listed.
The report indicates:
.NF
     shp#          ship number
     ship type     type of ship (cargo ship, destroyer, etc)
     Starting      The first sector to move and cargo hold levels.
     Ending	   The destination sector to and cargo hold levels.

shp#     ship type           [Starting]  ,  (Ending)
   6 cargo ship 1      [1-s:50 2-l:500]  ,  ( 1-g:50 )
  54 cargo ship 1                   [ ]  ,  (1-h:1000)
1003 destroyer 2               [1-s:40]  ,  ( )
.FI
This report reads.
Cargo Ship #6 will sail to its starting sector, displayed
in the sorder, and load cargo hold 1 with 50 shells and cargo hold 2 with 500
light construction materials.  Then sail to the ending sector, again
displayed in the sorder command, unload the cargo and load cargo hold
1 with 50 guns.
.s1
.SA "order, sorder, bmap, navigate, ship, Ships, Autonav, Autofish"
