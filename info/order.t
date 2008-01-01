.TH Command ORDER
.NA order "Order a ship to auto-navigate"
.LV Expert
.SY "order <SHIP/FLEET> [c|r|s|d|l] 
You use the \*Qorder\*U command to give sailing orders to ships.
At each update, ships sailing under orders \*Qautonavigate\*U toward a
specific destination sector.  Telegrams notify you of arrivals.
.s1
ONLY merchant ship may be given orders, but in order to move during the
update a ship must have at least one crew (which may be civilian or military,
but not an uncompensated worker).  Merchant ships are those that
have 0 for their firing range and gun limit values.  
.s1
Note that your ships sailing on orders will be interdicted just as if
you were navigating them by hand.
.s1
.EX order <SHIP/FLEET> c      Clear Orders
.s1
     Use the clear option to wipe out any orders the ship
currently has.  This will remove all starting and ending
points and clear all the cargo levels for the ship.
.s1
.EX order <SHIP/FLEET> s      Suspend Orders
.s1
     If you want to keep orders but don't want the ship
to move during the update use the suspend option. 
A quick look at 'sorder' 
will display the eta time as suspended.
As long as a ship is suspended it will be unable to move
during the update.  
.s1
.EX order <SHIP/FLEET> r      Resume Orders
     Resume will allow ships that have a suspended order to move
again. 
.s1
.EX order <SHIP/FLEET> d <dest1> [dest2|scuttle|-]   Declare Orders 
.s1
To declare an order you have 3 options.
.NF
     1) Order a ship to any sector on the map.  
	When it arrives it will clear its orders.

     2) Order a ship to any sector, load goods,
        move to a second sector and unload goods.
        Continue to loop between the 2 points.

     3) Order a ship to any sector on the map.
        When it arrives, it will scuttle itself if it is in a harbor that
          is at least 2% efficient.  This is useful for tradeships.

Example:  You have a cargo ship (#6) in your harbor at sector 6,0
You want to move good between that harbor and another harbor at
sector 14,0. 

 order 6 d 6,0 14,0

This reads:
 Cargo Ship #6, move to sector 6,0 load goods, sail to sector 14,0
 unload cargo then load again and sail back to 6,0.

Example: You have a tradeship (#666) at sea in sector -10,0 and you
want to move it to a friendly harbor at 20,4.

 order 666 d 20,4 -

The ship will now move to sector 20,4 with the most efficient path.

Example: You have a tradeship (#666) at sea in sector -10,0 and you
want to move it to a friendly harbor at 20,4, and then scuttle itself.

 order 666 d 20,4 scuttle

The ship will now move to sector 20,4 with the most efficient path,
and scuttle itself when done.

Note that only trade ships can be given auto-scuttle orders, and won't
scuttle themselves at sea.
.FI
.s1
.EX order <SHIP/FLEET> l <hold> <start/end> <COMM> <amount>
.s1
Set cargo Levels. 
.NF 
Example: Cargo ship #109 is at your harbor in sector 10,0.
You have agreed to trade 200 lcm and 100 hcm to your friend and
he will give you 100 shells each update.  His harbor is at -10,2.
Using 'order declare' you set up your start point 10,0 and end point -10,2

order 109 l 1 start lcm   200
order 109 l 2 start hcm   100
order 109 l 1 end   shell 100  

When your finished the output would look like this. 

sorder 109
shp #   type           x,y     start   end     len  eta
  109   cargo ship 2  10,0      10,0   -10,2    11   1

qorder 109
shp #   type           [Starting]  (Ending)
  109   cargo ship 2   [1-l:200 2-h:100 ] , (1-s:100 )

.FI 
.s1
AutoNav Features.
.s1 
Whenever a ship is autoloading cargo, it will always wait until it loads
to capacity.  If a ship does not load fully it will not move!!! 
Remember 'ships need food!!' unless the NOFOOD option is enabled.
Set one of your cargo holds to load food or your ship will starve at sea.
When unloading cargo, the ship will unload all goods
listed in the cargo holds ONLY!  It will not unload any extra good
manually placed on the ship.  
.s1
If you are dealing with civilians 1 will be left in either the
sector or the harbor depending if your loading or unloading.
In either case when you arrive at the other harbor all but 1 civ
will be dumped into the harbor.  Be careful if you set civilians
as one of your cargo levels. 
.s1 
If the FUEL option is enabled ships will try and fuel when they load
cargo in the starting or ending point and they currently have a fuel
level less then half of their max fuel level. 
Auto fueling is not foolproof.  If you declare a 
long path and the ship does not pass through any harbors it might
run out of fuel and mobility. Remember ships can only autofuel 
in harbors. 
.s1
You can set any commodity in the game to be loaded on the ship.
However if you tell a ship to load a commodity it can not hold it
will be ignored.  Example:  If you tell a battleship to load
Lcm, and of course it does not have that capability, see 'show ship 
capability' that level will be ignored at the update.
Planes and Units are not commodities so they can not be loaded this way. 
.s1 
At update time, the most direct path from the ship to destination
is calculated.  Each ship is then autonavigated along that path to
the best of its mobility.  The path is calculated from what the player
knows of the world (what you can see using the \*Qbmap\*U command).
Open sea, bridges, harbors, and unexplored regions of the world
are considered to be navigable.  Your friend's harbors and bridges
may be uses as long as the harbor >= 2% and the bridge >= 60%. 
Your ships will also try to avoid any mines you have declared on your
bmap.  'x' or 'X' characters.  Should your ship hit a mine the ship
will stop moving and its orders suspended.  You bmap will also be updated
with a 'x' marking the sector for you. 
.s1 
If a ship sails in range of enemy forts they will be fired upon.
Setting your ship to autonavigate into an area with good fort
cover could result in many ships getting sunk fast.
However you could put an invasion fleet off someone's coast in a hurry. 
.s1 
If the ship has a 'sweep' flag
ships will try and sweep any mines if any are present as they move.
.s1
Radar operates continuously as the ship moves along the path, constantly
adding sector information to the known world map.
.s1
The autonav code will try and use as much mobility as possible and this
is all done before mobility is gained from the update.
After an update has completed
your ship will have mobility so it can be moved by hand if needed. 
.s1
Ships with capability fish or oil can be setup to \*Qload\*U
from the sea sector.
See info \*QAutofish\*U for details.
.s1
.SA "sorder, qorder, bmap, navigate, ship, Autofish, Autonav, Ships"
