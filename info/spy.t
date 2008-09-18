.TH Command SPY
.NA spy "Snoop on adjacent enemy sectors"
.LV Basic
.SY "spy <SECTS>"
The spy command allows reconnaissance operations to enforce
an oral treaty or as preparation for an attack
(or even for a prayer to the Deity).
.s1
The spy report will reveal, (in round figures), the numbers of troops,
civilians, guns, shells, iron, and planes and the sector designation and
efficiency in neighboring sectors.
.s1
The sectors denoted by <SECTS>
are those from which you are spying,
(see \*Qinfo syntax\*U for format).
Note: you can only spy from sectors that have military or land units in them.
.s1
Note that the number of BTU's consumed
is dependent upon the number of sectors into which you spy,
e.g., a 5x8 area costs one full BTU.
.s1
If you are spying on an unallied country your spy could be apprehended...
his/her chances are dependent upon the number
of patrolling military in the sector.
.s1
However, having a recon unit in the sector the spy is coming from
enhances your chances of succeeding even if the spy is caught.
.s1
If the other country has declared itself at war with you the spy
will be shot and the other country, (and the rest of the world,
through the news), will be warned that you tried to spy on him...
.s1
If the other country is neutral towards you the spy will be deported.
.s1
.EX "spy 29,3"
.NF
SPY report
Thu Aug  8 10:48:18 1996
                 old sct rd  rl  def
   sect   de own own eff eff eff eff  civ  mil  shl gun  pet food bars lnd pln
   28,4  bf    2   2 100  90   0  50  100   30   90 100    0   40    0   0   0
   29,5  e     2   2   0   0   0   0    0    0    0   0    0    0    0   0   0
Neutral unit in 29,5:  cavalry #6 (efficiency 98, tech 18)
Neutral unit in 29,5:  if1  infantry 1 #7 (efficiency 98, tech 15)
.FI
.s1
Spies will also report all non-owned planes and land units that exist in
the sector you are spying from, such as allied land units that have been
marched there, allied planes that have been landed there, spy units that have
marched there and non-allied units that have been trapped there.
.s1
Note that spy units are not always seen.  There is a 10-100% chance of a spy
unit being seen by the spy command.  See "info Spies" for more information.
.s1
.L NOTE
.s1
Spies reporting on the tech level of enemy units are pretty inaccurate.
They can vary by +/- 20 tech or so...
.s1
.SA "census, read, declare, Sectors, Combat, Detection"
