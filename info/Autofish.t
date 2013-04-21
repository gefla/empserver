.TH Concept Autofish
.NA Autofish "How to exploit resources at sea automatically"
.LV Expert
Fishing boats and oil derricks let you exploit sea resources.
Navigating them back and forth between harbor and fishery / offshore
oil field is rather tedious.  Autonavigation lets you automate this.
.s1
It is more commonly used for fishing boats than for oil derricks,
because oil fields are commonly configured to deplete quickly, and are
often too far from harbors for the slow oil derricks to make
autonavigation effective.
.s1
This is an example of how to set up a fleet of fishing boats
so they will take their food produced each update and drop it off
at the harbor of your choice.  Oil derricks work exactly the same.
.s1
See info \*Qorder\*U for a complete description and example of
syntax for the \*Qorder\*U command.
.s1
.NF
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
.s1
You have ships 0,12 and 88 in the harbor at 15,1.
Sector 16,0 has a very good fert of 93 so you want to use that
as your fishing grounds.
.s1
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
          autofish will work correctly and set them at the
          same levels!

    NOTE: Fishing boats and oil derricks are a special case
          to the autonav code.  This setup will only work
          correctly with them.  If you want to use autonav
          with cargo ships please use the examples found in
          the \*Qorder\*U command.

repeat 2a,2b for other ships in the area.
.FI
.s1
During the update your fishing boats will now move to the harbor at
15,1 unload food until it reaches 30, then move back to 15,1 and STOP.
So unless the ship gets sunk, or something happens to it, like the
civs starve on board if you don't leave enough food on it,
the ships will continue to feed your country and you won't even need
to think about them.
.s1
For oil derricks, step 2b becomes:
.nf
    "order 0 level 1 start food 30"
    "order 1 level 2 end   oil 1"

    Note: If food is not required, food cargo levels can be zeroed.

    Note: In this configuration, the end oil level is not important
          as it is not used.
.fi
.s1
If your ship can't be unloaded because the the harbor is overcrowded,
it will continue along its route.
.SA "Autonav, order, navigate, Ships, Populace, Updates"
