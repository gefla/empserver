.TH Server "Merc Empire Changes"
.NA Merc "Changes from KSU code to Merc code"
.LV Expert
.s1
There have been several changes/fixes made to the KSU code for
the Merc V game.  This will outline the various changes and how
they will affect you.

Bridges

There was a bug which allowed planes that were on bridges that collapsed
to remain in the sea sector and could be retrieved by either rebuilding
the bridge or moving them onto a 100% highway.  This has been fixed so
that any planes upon a bridgespan which falls will be destroyed.

Uncompensated Workers

Uncompensated workers were not paying taxes, even though the version command
provided a tax rate for them.  They now pay the taxes that are shown.

.SY "reject reject announcements *"

This command now works, so that you can reject announcements from
everybody.  This is not recommended though...

Technology

The tech formula has been fixed so that all increases in material
result in an increase in the amount of tech produced.

.NF
	if(breakthroughs > 0.75) {
		above = breakthroughs - 0.75;
		tech = 0.75 + log2(1 + above);
	}
	else {
		tech = breakthroughs;
	}
.FI

Grind

The grind command now requires available work to grind up bars.
It takes 5 avail to grind up a gold bar.

Loans

Loans have been removed from Merc V.  This is an experiment to see
how things go without loans being available.

Country Names

Certain commands only worked with country numbers, but they now work
with country names as well.  The primary command that this affects for
Merc V is report.

Production

The production of dust/oil/rads is now limited to the resource content
of the sector.  It used to be possible to get more out of a sector.,

Starvation

Sectors that have a very large number of civilians that starve will
have at most 499 civilians after the update unless there was enough food
to feed more than 499 civs.

Attack

The attack command now uses at least 1 mobility point per sector that
participates in any attack.

Pinpoint Bombing

Pinpoint bombing now lists all the commodities in a sector that can
be bombed.

Production Report

The production report now contains a report of what will actually be produced
by your sectors at the next update.  Since some of the rounding done by the
update code uses a random percentage chance, it is possible that you will get
1-2 units more of some of the items.  The production report command is
conservative, and assumes that all rounding will be down.

Version

The information in the version command was modified so that the information
that was printed out would reflect the actual parameters of the game.


.SA "Server"
