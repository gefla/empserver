.TH Concept "Conditional Selectors"
.NA Selector "Selectors you can use in Empire conditionals"
.LV Basic
This is a complete list of all selectors you can use in Empire
conditionals.  A selector is for example \*Qmil\*U in the following
command:
.EX cen * ?mil>10
Most selectors are self-explanitory, so definitions will only be givin
where necessary.
.s1
Commodities:
.in +\w'nchoppers\0\0'u
.L civil
.L milit
.L shell
.L gun
.L petrol
.L iron
.L dust
gold dust
.L bar
bars of gold
.L food
.L oil
.L lcm
light construction materials
.L hcm
heavy construction materials
.L uw
uncompensated workers
.L rad
radioactive materials
.s1
.in
Sector:
.in +\w'nchoppers\0\0'u
.L xloc
.L yloc
.L owner
.L des
designation/infrastructure
.L effic
.L mobil
.L terr
.L timestamp
.L road
.L rail
.L dfense
(this isn't defense or it will conflict with des)
territory
.L work
.L coastal
is a coastal sector
.L newdes
new designation
.L min
mineral resources (iron)
.L gold
gold resources
.L fert
food fertility
.L ocon
oil content
.L uran
uranium resources
.L oldown
old owner
.L off
whether production is turned off
.L xdist
distribution centre
.L ydist
distribution centre
.L avail
available workforce
.s1
.in
Ship, plane, or land unit:
.in +\w'nchoppers\0\0'u
.L xloc
x location
.L yloc
y location
.L owner
.L type
type of unit (e.g. cavalry or frigate)
.L effic
.L mobil
.L timestamp
.L sell
.L tech
tech level the unit was built at
.L uid
unit id number
.L group
fleet/wing/army
.L opx
mission operations sector
.L opy
mission operations sector
.L mission
.nf
1 = interdict
2 = support
3 = reserve
4 = escort
5 = sub interdiction
6 = air defense
7 = defense support
8 = offense support
.fi
.s1
.in
Ship:
.in +\w'nchoppers\0\0'u
.L fleet
.L nplane
number of planes the ship is carrying
.L fuel
.L nxlight
number of xlight planes the ship is carrying
.L nchoppers
number of helecopters the ship is carrying
.L autonav
.s1
.in
Plane:
.in +\w'nchoppers\0\0'u
.L wing
.L range
.L ship
ship the plane is on (-1 = no ship)
.L att
.L def
.L harden
.L nuketype
Type of nuke on the plane (-1 = no ship, 0..n = index into nuke table)
.L flags
.L land
land unit the plane is on (-1 = no land unit)
.s1
.in
Land Unit:
.in +\w'nchoppers\0\0'u
.L att
.L def
.L army
.L ship
ship the land unit is on (-1 = no land unit)
.L harden
fortification
.L retreat
.L fuel
.L land
number of land units on the unit
.L nxlight
number of xlight planes on the unit
.s1
.in
Nuke:
.in +\w'nchoppers\0\0'u
.L xloc
.L yloc
.L number
.L ship
.L trade
.L timestamp
.s1
For distribution and delivery selectors, the first letter is the first
letter of the name of the commodity.  For example, civilian
distribution threshold is 'c_dist'.
.s1
.in
Distribution:
.in +\w'nchoppers\0\0'u
.L c_dist
.L m_dist
.L u_dist
.L s_dist
.L g_dist
.L p_dist
.L i_dist
.L d_dist
.L b_dist
.L f_dist
.L o_dist
.L l_dist
.L h_dist
.L r_dist
.s1
.in
Delivery:
.in +\w'nchoppers\0\0'u
.L c_del
.L m_del
.L u_del
.L s_del
.L g_del
.L p_del
.L i_del
.L d_del
.L b_del
.L f_del
.L o_del
.L l_del
.L h_del
.L r_del
.s1
.in
Deity:
.in +\w'nchoppers\0\0'u
.L pstage
.L ptime
.L che
.L fallout
.L lmine
.s1
.in
Treaty:
.in +\w'nchoppers\0\0'u
.L cna
.L cnb
.L status
.L acond
.L bcond
.L exp
.s1
.in
Loan:
.in +\w'nchoppers\0\0'u
.L loaner
.L loanee
.L sell
.L irate
.L ldur
.L amtpaid
.L amtdue
.L lastpay
.L duedate
.s1
.in
News:
.in +\w'nchoppers\0\0'u
.L actor
.L action
.L victim
.L times
number of times action happened
.L time
time 
.s1
.in
Trade:
.in +\w'nchoppers\0\0'u
.L type
.L owner
.L unitid
.L price
.in
.s1
.SA "Syntax, Playing"
