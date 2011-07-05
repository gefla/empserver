.TH Concept "Maintenance"
.NA Maintenance "The cost to maintain ships, planes, and land units."
.LV Expert
During each update, nations must pay to maintain their units, ships and planes.
.s1
The cost to maintain a ship, or land-unit is:
.NF
	.001 * cost of thing * ETUs/update
.FI
.s1
The cost to maintain a plane is:
.NF
	.001 * cost of thing * ETUs/update + 5 * mil_cost * crew
.FI
.s1
Note that this cost can be changed by the deity.
.s1
Also note that if you have purchased high-tech items, and your tech
is not at least 85% of the item's tech, you are charged double
to maintain the item.
.s1
Also note that engineering land units costs 3 times what
normal units cost, after the tech is figured in.
.s1
If you can not afford to pay maintenance costs then your unit will
lose ETU/5 points of efficiency.  It won't go below its minimum
efficiency, though.  Thus, if you were playing in a 60 ETU game, and
you did not pay your maintenance costs for a 40% frigate, then the
frigate would drop to 28% efficiency at the first update, and to 20%
at the second update.
.s1
.SA "Unit-types, Ship-types, Plane-types, Ships, LandUnits, Planes, Updates"
