.TH Concept Autofeed
.NA Autofeed "How to use fishing boats to automatically feed your civs"
.LV Expert
.NF

  For years now people have been setting up large groups of fishing
boats to feed their island.  These groups get very large and are
difficult to keep track of.

  This is an example of how to set up a fleet of fishing boats
so they will take their food produced each update and drop it off
at the harbor of your choice.

  See 'info order' for a complete description and example of
syntax for the 'order' command.

map #1
   00000000001111111
   01234567890123456
 -4. . . . . . . . .
 -3 ? ? . . . . . .
 -2? . . . . . . ) .
 -1 ? . . . . . f . .
  0. . . . . g m . .
  1 . . . . f * w h .
  2. . . . . c + + .

You have ships  0,12,88 in the harbor 15,1
Sector 16,0 has a very good fert of 93 so you want to use that
as your fishing grounds.

1) move your ships to sector 16,0
"nav 0/12/88 16,0"

2a) set up autonav using the 'order' command.
    "order 0 des 15,1 16,0"
    This will establish a circular trade route.

2b) set up food levels for the ship
    "order 0 level 1 start food 30"
    "order 0 level 1 end   food 30"

    You can also use a global '*' or a fleet letter here.
    "order * level 1 start food 30"
    "order * level 1 end   food 30"
   
    "order F level 1 start food 30"
    "order F level 1 end   food 30"

    NOTE: You should set BOTH start and end fields to insure
          autofeed will work correctly and set them at the 
          same levels!

    NOTE: if you are using autonav with normal cargo ships.
    Fishing boats are a special case to the autonav code.
    This setup will only work correctly with fishing boats.
    If you want to use autonav with cargo ships please use 
    the examples found in the 'order' command.

repeat 2a,2b for other ships in the area.

During the update your fishing boats will now move to the harbor at
15,1 unload food until it reaches 30, then move back to 15,1 and STOP.
If you're using fishing boats that require fuel, i.e. fb2
when the ship enters the harbor, and the fuel level is at 0, the
autonav code will automaticly refuel the ship for you.
So unless the ship gets sunk, or something happens to it, like the
civs starve on board if you don't leave enough food on it,
the ships will continue to feed your country and you won't even need
to think about them.

The Max limit that autonav will stockpile is 99999 units.
So if your sector get overcrowded the ship will sit in the harbor
until it can unload all of its food.
.FI
.SA "Autonav, order, navigate, Ships, Populace, Update"
