.TH Server "Empire2 Changes"
.NA Empire2 "Changes from the Chainsaw server to the Empire2 Server"
.LV Expert
.s1
There have been several changes/fixes made to the Chainsaw code in the
new Empire2 Server.  This outlines the various changes and how
they will affect you, the player.  Most of these changes were coded by
Ken Stevens <children@empire.net> and any bug-reports in the new
server should be directed to him.
.s1
.in 0
.\" -----------------------------------------------------------

Ships
.in 0.6in
.L -
Ships now have three letter abbreviations.
.L -
"load" syntax has been changed to make it the same as the "move"
syntax.
.nf
- The following changes were made to the sonar command:
  - will only work from ships at sea
  - will only detect ships at sea
  - will now detect the shoreline
  - will only detect things if there is a straight line of water
    between the detector and the detected
  - now displays a map and changes your bmap
  - for ships with tech > 310, shows mines on the map with an X and
    puts the X on your bmap
.fi
.L -
All ships (including subs) in harbors are now visible via the \*Qlookout\*U
command by all ships (including subs) in adjacent sectors
.L -
If a NEUTRAL (or worse) ship sails within your coastwatch range,
then you will receive a telegram like this:
.nf
TheBorg lc2 light cruiser 2 (#666) sighted at -9,-1
.fi
This telegram is sent before any interdiction is done.  If you
want to turn this feature off, then type "toggle coastwatch off".
Note that you can also now turn off those nuisance sonar ping
detection telegrams by typing "toggle sonar off".
.L -
You may now tend land units which have
"assault" capability (see info tend).
.L -
"build" syntax has been simplified.  You no longer need to specify tech
level to build at (the default is your current tech level).  Also, the
<NUMBER> arg now comes before the <TECH> arg:

.EX build s 4,0 fb1
builds a fishing boat at current tech level.

.EX build s 4,0 fb1 5
builds 5 fishing boats at current tech level.

.EX build s 4,0 fb1 2 99
builds 2 fishing boats at tech level 99.
.L -
"show" output now shows you the total amount of avail used to build something.
.L -
The Autonav code has been completely rewritten and is dramatically
improved.  Thanks Chad!
.L -
You now get a telegram if someone loads something onto your ship or if
someone unloads into your harbour.
.L -
Load/Unload now tell you exactly how much stuff got loaded/unloaded
from each ship.
.L -
You can now only unload into a sector if you are friendly towards them.
.L -
The "coastwatch" command now shows ship names!  (Empire invents the
telescope!)
.L -
The "payoff" command now shows the distance your ship has
traveled--handy for making sure you pass those critical distances.
.L -
The tech level required to build aegis cruisers has been increased
from 150 to 260.
.L -
The following ships: "mc  missile cruiser", "aac aa cruiser", "agc
aegis cruiser", and "ncr nuc cruiser" have been given a new ship
capability called "anti-missile"  This means that they will use their
anti-missile systems to attempt to
intercept enemy missiles aimed at any ship within a 1 sector radius.
Note that just like abm & a-sat missile interception, only missiles
belonging to countries you are "Hostile" or "At War" with will be intercepted.
Thanks Chad!
.L -
Ship damage now always factors the armor value of the ship, no matter
where the damage comes from.  Also, whenever a ship is hit, the player
is told exactly how hard that particular ship was hit.
.L -
The "scuttle", "scrap", "mission", and "fuel" commands will now accept
"l" for the unit type.  (You are no longer required to type "la".)
.L -
Now when you load land units onto a ship, the code will automatically
try to supply them, and if they are out of supply, you will get a warning.
.in 0
.\" -----------------------------------------------------------

Subs
.in 0.6in
.L -
ASW planes will now fly out to track any non-FRIENDLY (including
NEUTRAL) sub movement.
.L -
However there is a chance that the plane won't even leave the ground
and notice any sub movement at all.  The chance of detection is
identchance = (1 - (1 - hitchance)^2).
.L -
ASW planes will no longer detect subs they fly over while flying an
interdiction mission.
.L -
Once the planes have arrived at the sector where the sub movement
was detected, then identchance is checked again.  If success, then
the plane is able to identify the owner of the sub.  If the owner
is neutral, then the player gets the message:
.nf
      np1 naval plane 1 #2 tracks TheBorg subs at 2,-6
.fi
If the plane is unable to identify the owner of the sub, then the
player gets the message:
.nf
      np1 naval plane 1 #2 detects sub movement in -3,-3
.fi
If the ship was identified as HOSTILE or AT_WAR, then the plane will
drop depth charges.
.L -
If depth-charges miss, then the ship owner will get
the message:
.nf
RUMBLE... your sub in 24,0 hears a depth-charge explode nearby
.fi
.L -
torp and fire will now only tell the victim that a "sub" torped
them, or that return fire damaged a "sub" (rather than giving out
detailed information about the sub number, type, and owner...)
.L -
max sonar range has been reduced to 7 and sonar output no longer
prints blank lines.
.L -
sonar has been changed in the following way:
detection = (source "spy" value) + (target "vis" value)
If the detection value is less than 8, then sonar will just print
.nf
  Sonar detects sub #12
.fi
If the detection value is above 7, then sonar will report the type of sub.
Only if the detection value is above 9 will the owner of
the sub be revealed.
.L -
For ASW planes flying "sonar" or "recon" missions, a similar change
has been made.  The plane needs to pass one identchance check to see
the type of sub, and needs to pass two identchance checks to see the
owner of the sub.
.L -
Note in both of the above cases, owner and type of FRIENDLY and
ALLIED subs will always be revealed.
.in 0
.\" -----------------------------------------------------------

Navigation
.in 0.6in
.L -
When you "nav" or "order" your ships, they will automatically avoid "x" and "X"
sectors on your bmap.
.L -
now when you navi (or march), any new sector designations
automatically appear on your nav map and get added to your bmap (you
no longer need to do a "look" to find out their designation.  Note
that subs will only see sea, harbors, and bridges and will have to
look" (and risk detection) to see anything else.
.L -
If you hit the wrong character, navi will now display all possible navi
commands.
.L -
You can now draw a 'M' map or a 'B' bmap while navigating.
.L -
You can list the ship you are navigating using 'i' and you can change
your flagship using 'f'.
.L -
Navigating using "nav", "order" or "sail" now calls exactly the same
code and will thus result in exactly the same risks (mines & interdiction).
.L -
Now you can type commands like this:
.EX nav 72 jlsruMluBursh
That is, you can mix directions with other navigation commands.
.L -
Interdiction of navigating ships now works as follows:
.NF
1. Forts interdict.
2. If there are any ships left, ships, land, planes interdict.
3. If there are any ships left, marine missiles on interdiction  fire one at a
   time until there are no ships left.
.FI
.L -
There is now a new kind of diplomatic relation that you can have with
a country: "Friendly" which falls between "Neutral" and "Allied".
This basically splits "Allied" into two steps: anything to do with
ships is available to "Friendly" nations (nav, load, unload, tend,
upgrade, order, sail, follow), whereas anything to do with planes or
military support is reserved for Allies.  This allows you to
establish trading relationships without the necessity of a military
relationship.  See info relation for full details.
.L -
If the BIG_CITY option is enabled, then small ships may navigate,
load, and unload in 2% cities.
.in 0
.\" -----------------------------------------------------------

Planes
.in 0.6in
.L -
Planes which are less than 40% efficient can not fly.
.L -
If you're tech is higher than 290, then you may not build planes with
tech lower than 150.
.L -
Planes will only abort if they have been damaged below 80% (it used to
be that they could abort at anything below 100%).
.L -
The range of marine missiles and SAM's has been reduced.
.L -
Air combat reports are now succinct tables with headers.  Much easier
to read.  Thanks Waffen!
.L -
New command "skywatch" scans your skies overhead for satellites.
Works very much like "coastwatch".
.L -
You can now filter your satellite output to list, for example, only
banks owned by country #7 which are over 60% efficient.  The selectors
available are exactly the same as those used by the "census", "ship",
and "land" commands.  See "info satellite" for more details.
.L -
A 75% satellite now only sees 75% of sectors (the rest are blank).
It's always the same 25% that are blank.
.L -
All missiles are now VTOL.  In particular, this means that missiles on
missions will now fire from anywhere (including on subs).
.L -
Surface missiles can now be put on interdiction mission.  To
distinguish surface missiles from anti-ship missiles, the latter have
been given a new capability called "marine".  Surface missiles on
interdiction will keep firing on the enemy until at least 100 damage
has been done.
.L -
Interdicting marine missiles will also fire on ships when they "fire".
.L -
Missiles and planes flying interdiction missions will now detonate
any nuclear warheads that they are carrying.
.L -
When you "arm" a missile or plane, you will be asked whether you
would like to program the warhead to airburst or groundburst.
.L -
Missiles and planes are now limited to a maximum of one warhead
apiece.
.L -
A-sats will now automatically attempt to intercept any satellites
launched by a country you're "Hostile" or "At War" with.  This works exactly the same
way as ABM defense.  Also, if you launch a satellite over a sector
which is owned by a non-ally then they will get a message telling them
that you have positioned a satellite over their sector.  In the age of
advanced telecommunication such things don't go unnoticed.
.L -
There is now only one formula for determining whether a plane/missile
hits its target and how much damage it does (including abm's and
a-sat's).  See "info bomb" for the formula.  In particular,
planes/missiles flying missions will now have exactly the same
hitchance as those flown by hand.
.in 0
.\" -----------------------------------------------------------

Land Units
.in 0.6in
.L -
Land units now have three letter abbreviations.
.L -
Artillery must be at least 20% efficient before it can fire.
.L -
The "lmine" command now allows you to lay as many land mines as you
want.  If the unit can't carry as many mines as you asked it to lay,
then it will just keep resupplying itself until it finishes the job.
Also, it now costs engineers 1 point of mobility per mine laid.
.L -
If land units are accompanied by engineers, then their chance of
hitting a land mine is reduced by a factor of 3.  E.g. if there 
are 25 mines in the sector, then a land unit has a 33%
chance of hitting a land mine.  If it is accompanied by an 
engineer, then that chance drops to 11%.
.L -
If an engineer sweeps a land mine that it cannot carry, it drops
it as a shell into the sector.
.L -
Engineers now sweep a number of times equal to 2 times the number
of shells they can carry.
.L -
The chance of an engineer finding a land mine in a sector is now
50% time the "att" value of the unit:
.nf
          # of sweeps   chance of finding a mine
old system   5            66%
eng1  130    6            60%
eng2  165    8            75%
meng  260    8            90%
.fi
.L -
You can now draw a 'M' map or a 'B' bmap while marching.  Also, if
you hit the wrong character, navi will now display all possible march
commands.
.L -
Fortifying a land unit no longer clears the unit's mission.
.L -
You can list the units you are marching using 'i' and you can change
your leader using 'f'.
.L -
Now you can type commands like this:
.EX mar 72 jlruMluBurh
That is, you can mix directions with other march commands.
.\" -----------------------------------------------------------
.in 0

Forts
.in 0.6in
.L -
Forts now get their range bonus for return fire if they're > 60%.
.L -
Forts now do not cost anything to designate.
.L -
You may not fire a fort which is less than 5% efficient.
.L -
Forts will now only automatically interdict ships that the country
would be able to see using the "coastwatch" command.  Ships out of
visual range will not be interdicted by forts.  Note that
ship/ship, unit/ship, plane/ship interdiction was not changed,
and also the "fire" command was not changed.  _only_ fort/ship
interdiction was changed.
.in 0
.\" -----------------------------------------------------------

Sectors
.in 0.6in
.L -
Bank interest is now scaled by efficiency rather than simply turning
off below 60%.
.L -
Shoot syntax has changed from "shoot 2,0 c 400" to "shoot c 2,0 400".
.L -
Harbors now have the same packing bonus as a warehouse.
.L -
You can now build bridges all on one line as in:
.EX build b 2,2 j
.L -
You are now only allowed to cede to nations which are Friendly towards you.
.L -
The new Empire command "strength" will show you the defense strength
of all your sectors (including number of land mines and reacting units
from nearby sectors).
.L -
The "test" command no longer requires there to be the commodity in
the sector (useful if you want to use that sector as a temporary
stopover and you don't want the stuff to get stranded there).
.L -
Shelling now factors in the efficiency of the sector into its
defensive bonus, so now a 0% bank defends the same as a 0% fort which
defends the same as a 0% road.  The formula for calculating the new
defense value of a sector is:
((defense - 2)*eff/100) + 2
.L -
The "neweff" command now checks to see if a sector has been stopped.
.L -
xdist and ydist are now in player coordinates.
.L -
Any interdiction of things being moved will also damage the sector
being moved into.  Note: this will slow down bridge attacks!
.L -
Deliver now gets the same 4x bonus that distribute gets.
.L -
The "spy" command now gives you much more useful information:
.in 0
.NF
SPY report
Wed Jul 26 21:10:13 1995
   sect   type own oldown eff  civ  mil  shl gun  pet food bars units planes
  29,5      c+   4      4  90  980    0  940  10    0    0    0     2      0
.FI
.in 0.6in
.L -
A new option has been added to the server called BIG_CITY.
When the BIG_CITY option is turned on, it
takes lcm's, hcm's, and money to create a city ('c' sector), but they
can hold ten times as many civs, and get a 10x packing bonus for moving civs.
.L -
It used to be that a gold mine could not hold more than 999 dust.  Now
all sectors are allowed to hold 9999 of anything, but it is now only
_production_ which is limited to 999 per update.
.L -
Now the "starvation" command will let you check for starvation on land
units or ships.
.in 0
.\" -----------------------------------------------------------

Info
.in 0.6in
.L -
The info pages are now organized into subjects.  Type "info" for a list.
.L -
The budget command has been changed so that now you may choose not to
pay maintenance costs.  Also, "mil" on ships/planes/units has been
moved from the maintenance costs for those items to the "Military" section
of the budget.
.in 0
.\" -----------------------------------------------------------

Commands
.in 0.6in
.L -
Broke countries are now allowed to use all commands except for the
following:
.s1
arm assault attack board bomb build convert disarm drop enlist explore
fire fly grind harden launch lmine mine paradrop recon satellite scrap
shoot sweep torpedo treaty upgrade work
.L -
Broke countries used to be prevented from using the following
commands, but will now be allowed to use them:
.s1
demobilize lload llookout load lookout lradar lrange lretreat ltend
lunload march mission morale mquota navigate order qorder radar range
retreat sail sonar sorder spy supply tend transport unload unsail
.in 0
.\" -----------------------------------------------------------

Update
.in 0.6in
.L -
If you can not afford to maintain your ships/planes/units or you
choose not to maintain them using the "budget" command, then they will
lose ETU/5 % efficiency at the update.  Also, if you are broke at the
point of the update when it comes time to build something, then that
thing will not be built.  In short, the only way you can go broke now
is by having lots of mil to pay.
.in 0
.\" -----------------------------------------------------------

Communication
.in 0.6in
.L -
The new commands "flash" and "wall" permit instant communication
between friends and allies respectively.  Type "info flash" and "info
wall" for more details.
.L -
The new command "toggle" can be used to instantly inform you when
you receive a new telegram.  You can also use "toggle" to temporarily
block flash messages.  To find out which clients support this, type
"info Clients".
.L -
The server will now correctly report how many telegrams you have
waiting.
.L -
The new command "pray" lets you send a telegram to the Deity.  Also
a telegram from the deity is no longer called a BULLETIN, it is now
called a TELEGRAM.
.L -
The new command "motd" lets you see the "message of the day" (login
message) again.
.L -
The buggy commands "mail" and "wmail" have been removed from the
server.  If you want to use them, you'll have to use pei.
.L -
You can now specify which countries you'd like to see in your "power"
report.
.L -
New "report" command lists number ranges rather than "advanced",
"superior", etc...
.in 0
.\" -----------------------------------------------------------

Combat
.in 0.6in
.L -
Defense against attack, assault, and paradrop is now exactly the same.
.L -
If attack odds are > 90%, then defensive support is no longer called.
This will put a stop to most shell-draining techniques.
.L -
You can now specify how many troops you want to board with.  Also, the
code now always forces you to leave at least 1 mil on your ship when
boarding.  You can also board from any sector type and with land units
having "assault" capability.  You may not board with more mil or units
than the boarded ship can hold.
.L -
You may not attack with supply, security, artillery, or
anti-aircraft (flak capability) land units.
.L -
Attacking land units & mil used to be taken out of the sectors/ship
they were attacking/assaulting/boarding from.  This caused endless problems
(including stranded units).  In the new code, mil and units stay where
they were until they have finished attacking.  The battle is assumed
to take place along the "border" between the two sectors.  Thus, the
attacking mil & land units will be vulnerable to shelling and
attacking while you are answering the attack prompts.
.L -
If, while you are answering the attack prompts, anything happens to
either the sector you are attacking, or the mil & units you are attacking
with, then you will be notified ("land unit #3 damaged from 80% to 50%",
"Your mil in 3,1 damaged from 180 to 12", "defending mil has been increased
from 10 to 250").
You will also be notified in this way if a bridge collapses, one of your
attacking sectors was taken, you were shelled, your enemy abandoned
the sector you were attacking, your land units reacted to fight another
battle, ...)  You will be notified if any of these things change before the
battle and given a chance to abort your attack.  If for some reason it
is no longer possible to attack (e.g. you now own the sector, or a
bridge collapses) then the attack will be automatically aborted.
.L -
The sub-prompt for both the "attack with" and the "move in" questions
has changed to [ynYNq?].  The meanings of these 6 answers are:
.NF
y - yes this unit
n - no this unit
Y - yes all units in this army
N - no all units in this army
q - quit attack
? - print this help message
.FI
.L -
The server will now add up the mil you saw and the enemy land units your
scouts saw, and based on this info, it will estimate a defense strength of the
enemy.  If your scouts didn't see the efficiency of the enemy units, then
it will assume that they were at 100%.  Based on this estimated defense
strength, the server will calculate an estimate of your success odds for
you and if these
estimated odds are less than 50%, then the server will ask you if you're
sure you want to continue with the attack.  These estimates are
made before support is calculated, and before the enemy reacting
units have reacted, so it will not be particularly accurate.  However, if
by the time you've finished answering all the attack prompts something has
happened which has made your odds much worse than you expected (e.g. your
cavalry got shelled or your enemy beefed up his mil), then this will give
you a chance to abort the attack.
.L -
Immediately after a battle is won (i.e. before any move-in sub-prompts
are asked), 1 mil (first choice) or 1 land unit (second choice) is
automatically moved into the taken sector.  Not only does this put a
definitive stop to shell-draining tricks, but it also resolves what was a
big problem in the old code of the taken sector being owned but not
occuppied for a period.  This problem resulted in, among other things,
stranded land units.  It is impossible to strand a land unit in the new
code.
.L -
I have added a new option to the server called INTERDICT_ATT,
which, when enabled, calls the usual interdiction routines against
mil & land units moving into the victim sector after an attack.
The same chances of interdiction apply as in "move" and
"march".  The mil get interdicted sector by sector, and the
land units all at once.
.L -
In the new attack code, it is much harder for a unit to fail a morale check.
The new chance (recalculated after each hit) is (retreat% - unit efficiency).
Thus, a land unit which has a retreat percentage of 60% and is now 50%
efficient, will have a 10% chance of retreating with each hit.
.in 0
.\" -----------------------------------------------------------

Important bugfixes
.in 0.6in
.L -
Missiles on subs will now fly missions against enemy planes (in
chainsaw, the missiles simply vanish).
.L -
Subs now only interdict subs if they have sub-torp ability (in
chainsaw, sb1's will interdict sb1's).
.L -
Missiles will no longer hit subs.
.L -
Your airport now needs to be at least 60% eff in order to fly bombing
runs from it (in chainsaw you can bomb from 40% airports, but only fly
and recon to 60% airports).
.L -
You can't sweep, hit, or lay sea mines in a harbour.  You can't sweep,
hit, or lay land mines on a bridge.  (In chainsaw, your ally can hit
your land mines in your harbour.)
.L -
It is no longer possible to accidently go broke through any of the
following commands: build, convert, demob, des, harden, upgrade.  (In
chainsaw it's easy to accidentally go broke through one of these commands.)
.L -
Much of the "attack" function was rewritten to check for "special
cases" which can result in lost military and units.  Thanks Chad!
.L -
The range of missiles on interdiction mission was half
of what it should be.  this is now fixed.
.L -
If you typed "realm #2 0:10,0:10", it would change realm
#0.  This is now fixed
.L -
If you downgraded your relations from Allied to Neutral,
then the other country would downgrade their relations towards you to
Hostile.  This is now fixed.
.L -
You used to have to log out and back in again to stop being broke.
Now your status will be updated as soon as you get the money (well,
one command after to be exact).
.in 0
.s1
Options
.NF
The following options have been made standard:
DROPANY:	you can drop things on sectors belonging to allies
REJECTS:	you can reject announcements and teles from annoying countries
BMAP:		map info gained is saved in a big world map (bmap command)
AUTONAV:	ships can move automatically
SCUTTLE:	you can sink your own ships automatically
CONVASAT:	anti-satellite missiles don't need nuclear warheads
ABM:		anti-missile missiles exist
GRIND:		you can grind gold bars back into dust
NONUKESUB:	nukes don't affect submarines
RANGEDIT:	you can modify the range of your planes
MISSDEF:	ships with tactical missiles will fire them in defense
SHIPSAM:	ships can carry and use SAMSs
BIND:		network stuff, no interest to players (NITP)
MERC:		a collection of bug fixes
TACARGO:	planes that are cargo AND tactical can bomb (zeppelins)
NOGODS:		deity countries don't show up on power reports
BETTERARMOR:	armor has more of an effect on ship damage
GODNAMES:	bulletins are identified by which deity sent the
MULTIFIRE:	multiple ships/sects can fire at once
SHIPTORP:	ships can have torp ability
SWEEP_PLANES:	some planes can sweep mines
MINE_PLANES:	some planes can lay mines
ASW_PLANES:	some planes can detect/kill subs
HARBOR_POLICE:	you can board ships in your harbor using mil from the harbor
WING_LOAD:	you can load groups of planes on ships with 1 command
ALLYUPGRADE:	allies can upgrade your ships in their harbors
ALLYHARB(etc):	allies will increase your ships' efficiency in their harbors
RETREAT:	ships can automatically retreat, under conditions you set
STEALTHV:	stealth is a % chance to evade, all planes are rated 0-100%
SEMILAND:	some ships can land 20% of mil instead of 10% or 100%
WIRE:		announcements are seperated from teles/bulletins
STOP:		Sectors taken over must be started.  Start and stop are enabled

The following options have been permanently turned off
REALM_IN_NAT:	realms are displayed by the nation command
DAVYJONES:	nukes hitting sea affect 1 sect, hitting waste/sanct don't det
BABY_NUKES:	Nukes affect only 1 sector
.FI
.s1
.SA "strength, tend, toggle, flash, relations, arm, starvation, Damage, report, power, Clients, Attack, Server"
