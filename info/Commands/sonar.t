.TH Command SONAR
.NA sonar "Use sonar to look for ships"
.LV Basic
.SY "sonar <SHIP/FLEET> <brief?>"
The sonar command allows ships to look for submarines
and submarines to look for ships.
.s1
If the <brief?> argument is
specified then the sonar map will not be drawn.  This is useful if you
just need to see the list of ships you can detect.  Whether or not a
map is printed, the sonar information will be added to your bmap.
.s1
Only ships at sea may use sonar and sonar only detects ships
which are at sea.  Furthermore, there must be a straight line of water
between the detector and the detected (sonar doesn't work around corners).
.s1
In the current implementation, sonar works similar to 
lookout.  The major difference is that a submarines cannot use
lookout to sight ships.  Likewise, submarines cannot be sighted
by ships via lookout.
.s1
There are two aspects to sonar - passive and active sonar.
.s1
ACTIVE SONAR
.s1
This is the most visible form of sonar.  It is simply a
ship's ability to use sonar pings to detect other ships.
The range of a ship's active sonar is identical to the
range that it can sight ships via lookout.
(see \*Qinfo lookout\*U)
When you type "sonar 25", you are using active sonar.
.s1
PASSIVE SONAR
.s1
This is more subtle.  Passive sonar is a ship's ability
to hear sounds, mainly active sonar pings.  When a ship
pings you with active sonar, you can detect that ping.
If a ship with sonar gets pinged, that country receives
a telegram saying "Sonar ping from x,y detected by ship Z!"
In its current implementation, passive sonar is the ability
to detect a ping.
.s1
When a ship emits a ping, that ping has to travel out to a
target ship, reflect off of it, and then return to its source.
So, in order to detect a ship X sectors away, a sonar ping
must be able to travel 2X sectors - from the source to the
target, and back to the source.  So, if a ship can use active
sonar to detect ships X sectors away, passive sonar can detect
active sonars from 2X sectors away.  So the range at which you
can detect sonar pings is twice the range at which you can use
sonar.
.s1
The active and passive sonar modes means that you want to be
careful when you use sonar, because you will tell others where
you are long before you actually detect them.  This also gives
you some warning before you get your submarine attacked, as
enemy ships have to use sonar to find you.  The key to submarine
warfare is "run silent", meaning use your sonar only when you
have to.
.s1
On the other hand, destroyers are designed to hunt subs.  A
destroyer guarding a player's coast should use sonar often,
especially if it has other destroyers nearby to support him if
he gets in trouble.
.s1
For example:
.EX sonar 3
.NF
sb2 submarine 2 (#3) at 24,0 efficiency 76%, max range 2
Sonar detects TheBorg (#5) bb4 battleship 4 (#1) @ 23,-1
  .
 . B ?
. . 0 ?
 . . .
  . . .
.FI
.s1
Sonar-capable ships of tech 310 or better can also detect mines with
their sonar, although at only half their normal sonar range.  Mines
will show up on the sonar map as 'X' and this 'X' will be added to
your bmap.
.s1
In the case of using sonar to detect subs, detection = (sub visibility + ship
spy value).  If detection is less than 8, then the sonar will just
print you something like:
.NF
  Sonar detects sub #12
.FI
If detection is above 7, then you will be told the ship type, and if
it is above 9, you will also be told the owner of the sub.  Note that
type and owner will always be shown for Friendly subs.
.s1
.TH Concept plane-sonar
.s1
Planes with the ASW ability can use 
sonar while on recon missions. See info recon for more details.
.s1
.SA "coastwatch, Ship-types, Sector-types, radar, recon, Ships, Detection"
