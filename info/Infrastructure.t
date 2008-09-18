.TH Concept "Infrastructure"
.NA Infrastructure "Making your sectors better"
.LV Basic
.s1
INFRASTRUCTURE
.s1
The infrastructure of your sectors determines how well your sectors
defend, and how well you can move commodities and vehicles (land units)
through your sectors.
.s1
You can improve the "road", "rail" and "defense" efficiency of all of your
sectors, given that you have the correct commodities, enough mobility and
enough money.
.s1
The "road" infrastructure determines how well (i.e. the mobility cost)
commodities and non-train vehicles travelling through your sector are able
to move at both distribution time and while you are moving them by hand.
.s1
The "rail" infrastructure determines how well (i.e. the mobility cost)
trains are able to move through your sectors.
.s1
The "defense" infrastructure determines how well (i.e. the defense factor)
your sector defends itself.  There is a maximum amount that the sector
may obtain.  Use the "show sect stats" to see what the maximum values are.
.s1
So, what you can do, by using up some commodities, mobility and cash,
is to create a pseudo-highway network through non-highway sectors.  You can
still create highway sectors for just the designation fee of the sector,
but it costs you a whole sector to use it.  This is a trade-off for
both being able to use the sector and still move things through it at a
reasonable cost.
.s1
When you "improve" the infrastructure of your sectors, you are improving on
the efficiency of part of that sector (either road, rail or defense.)  By
improving it, it's statistics get better.  For roads and rails, that means
that the mobility cost to move through goes down.  For defense, that means
that you are building up the "defenses" of the sector towards the maximum
allowed (see "show sector stats" for the maximum values.)
.s1
Since infrastructure is on an efficiency basis, it takes damage just like
regular efficiency.  So, when a sector is shelled or bombed, the infrastructure
there is damaged in an efficiency based manner.
.s1
Infrastructure is also not affected by rebuilding the sector.  So, once
you build infrastructure, it stays in place until the sector is damaged in
some way.
.s1
Use the "sinfrastructure" command to see what the current infrastructure of
your country looks like.
.s1
Use the "improve" command to improve the infrastructure of your sectors.
.s1
To find out the required amounts of materials, mobility, and money
use the "show sect build" command.
.s1
Also, defensive infrastructure is not always improvable.  See the version
info to see if the DEFENSE_INFRA option is enabled or not.  If it is not
enabled, the defense infrastructure of the sector is always the same
as the efficiency of the sector.  If it is enabled, you must improve your
sectors defensive values using the "improve" command.
.s1
.SA "improve, sinfrastructure, Mobility, Sectors"
