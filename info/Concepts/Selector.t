.TH Concept "Conditional Selectors"
.NA Selector "Selectors you can use in Empire conditionals"
.LV Basic
This is a complete list of all selectors you can use in Empire
conditionals.  A selector is for example \*Qmil\*U in the following
command:
.EX cen * ?mil>10
Most selectors are self-explanitory, so definitions will only be given
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
.in
.s1
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
.L terr0
alias for terr
.L terr1
.L terr2
.L terr3
.L timestamp
.L fallout
.L road
.L rail
.L dfense
(this isn't defense or it will conflict with des)
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
.L ocontent
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
.in
.s1
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
.in
.s1
Ship:
.in +\w'nchoppers\0\0'u
.L fleet
.L nplane
number of planes the ship is carrying
.L fuel
.L nxlight
number of xlight planes the ship is carrying
.L nchoppers
number of helicopters the ship is carrying
.L autonav
.in
.s1
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
.in
.s1
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
.L vul
.L spd
.L vis
.L spy
.L rad
.L frg
.L acc
.L dam
.L ammo
.L aaf
.L fuelc
.L fuelu
.L maxlight
.in
.s1
Nuke:
.in +\w'nchoppers\0\0'u
.L owner
.L uid
.L xloc
.L yloc
.L number
.L types
.L timestamp
.s1
For distribution and delivery selectors, the first letter is the first
letter of the name of the commodity.  For example, civilian
distribution threshold is 'c_dist'.
.in
.s1
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
.in
.s1
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
.in
.s1
Deity:
.in +\w'nchoppers\0\0'u
.L pstage
.L ptime
.L che
.L che_target
.L mines
.in
.s1
Treaty:
.in +\w'nchoppers\0\0'u
.L cna
.L cnb
.L status
.L acond
.L bcond
.L exp
.in
.s1
Loan:
.in +\w'nchoppers\0\0'u
.L loaner
.L uid
.L loanee
.L status
.L irate
.L ldur
.L amtpaid
.L amtdue
.L lastpay
.L duedate
.in
.s1
News:
.in +\w'nchoppers\0\0'u
.L actor
.L action
.L victim
.L times
number of times action happened
.L time
time
.in
.s1
Lost:
.in +\w'nchoppers\0\0'u
.L owner
.L uid
.L type
.L x
.L y
.L timestamp
.in
\" Commented out, because there's currently no way to use these:
.ig
.s1
Commodity:
.in +\w'nchoppers\0\0'u
\" FIXME incomplete
.s1
.in
Trade:
.in +\w'nchoppers\0\0'u
.L type
.L owner
.L unitid
\" FIXME incomplete
.in
..
.s1
.SA "Syntax, Playing"
