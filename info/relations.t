.TH Command RELATIONS
.NA relations "Shows diplomatic relations between countries"
.LV Basic
.SY "relations"
The relations command displays your diplomatic status with other nations.
If the optional argument is specified, the command displays that country's
diplomatic status with other nations.
.s1
The data displayed by the nation command is formatted as a single page
report.
.EX relations
.NF

	MGM Diplomatic Relations Report       Sat Jun 17 23:24:46 1989

	0) POGO                  Allied    Allied
	1) 1                     Neutral   Neutral
	2) Groonland		 At War    Hostile

.FI
.EX relations 2
.NF

	Groonland Diplomatic Relations Report       Sat Jun 17 23:24:46 1989

	0) POGO                  Neutral   Neutral
	1) 1                     Allied    Neutral
	3) MGM			 Hostile   At War

.FI
.s1
Relations are: \*QAllied\*U, \*QFriendly\*U, \*QNeutral\*U,
\*QHostile\*U, and \*QAt War\*U. Each has certain ramifications.
.s1
.L Allied
Your ally is someone you trust deeply, and are willing to make
sacrifices for.  If you are allied with someone, then your forts,
ships, planes, & artillery will support them in battle.  Also, they
can over-fly you, and you will not be told.  (I.e. an ally can map you
out easily).  When you use the "wall" command, it sends a flash
message to all of your allies (see info wall).  Unless the ALL_BLEED
option is in effect, you will benefit from tech produced by your
allies, but not by others .  Allied nations are assumed to have
"friendly" trade relations.
.s1
.L Friendly
When you accept a country as a trustworthy trading partner, then you
establish friendly relations towards them.  This allows them to sail
ships into your harbours, load and unload goods, and have their
ships repaired there.  You are allowed to "tend" friendly
ships.  You may send "flash" messages (see info flash) to friendly nations.
You will be warned, however, if they overfly
your territory.  And if they do it enough times, your diplomatic
relations towards them will be automatically downgraded.  You may
"spy" on a friendly nation with no consequences.
.s1
.L Neutral
You start out neutral towards all countries.  It simply means that you
are ambivalent towards them.  If they overfly your territory, you will
be warned and your relations will automatically downgrade.  Caught
spies will be deported.
.s1
.L Hostile
If you are suspicious that a nation may be planning to attack you,
but you are not committed to an all out war against them, then you
should declare hostile relations towards them.  This will ensure that
your planes will intercept theirs if they
try to over-fly you, and your forts will fire on their ships when they
sail within range.    Also, your a-sat's and abm's will attempt
to intercept their satellites and missiles independent of the target
sector they have launched them at, and your ships with anti-missile
capability will attempt to intercept any incoming marine missiles
within range.  Caught spies will be shot.
.s1
.L War
If you are committed to expending major resources against a nation,
then you should declare war on them.  This will cause all of your
ships, artillery, planes and missiles which have been put on a mission
to react to enemy movement.
.s1
Also, there is an automatic progression. If someone you are allied with
attacks you, you become hostile towards them. If someone you are neutral
to over-flies or attacks you, you become hostile towards them. Neither of
these progressions can lead to war, however, they both stop at hostility.
.s1
.SA "Diplomacy"
