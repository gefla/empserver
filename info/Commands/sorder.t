.TH Command SORDER
.NA sorder "Show ship order statistical information"
.LV Expert
.SY "sorder <SHIP/FLEET>"
.s1
\*Qsorder\*U shows some statistical information on ships that
have been given orders.
The report indicates:
.NF
     shp#        ship number.
     ship type   type of ship (cargo ship, destroyer, etc)
     x,y         ships current position
     start       The first  destination for the ship.
     end         The second destination for the ship.
     len         The number of sector required to get from its
                 current position to the start sector.
     eta         The estimated number of updates required to
                 reach the start sector.

shp #    ship type     x,y   start   end   len  eta
   #6  cargo ship 1    6,0    2,0    20,0   2    1
  #10  destroyer 1   -10,0   20,2           17   2
.FI
This report reads:
Cargo ship #6 is currently at sector 6,0 and has orders to sail to
sector 2,0.  It will arrive and load any cargo set for that ship,
see qorder, and then sail to sector 20,0.  It currently needs to
sail 2 sectors and will arrive in 1 update at sector 2,0.

Destroyer 1 is at sector -10,0 and only has orders to sail to sector
20,2.  It needs to move 17 sectors and will arrive at 20,2 in 2 updates.
When it arrives order will be cleared since only one destination
order was given to it.
.s1 
.SA "order, qorder, bmap, navigate, ship, Ships, Autonav, Autofeed"
