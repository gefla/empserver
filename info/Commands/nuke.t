.TH Command NUKE
.NA nuke "List all nuclear \*Qdevices\*U in a given area"
.LV Expert
.SY "nuke <SECTS>"
For each sector in <SECTS>, this command prints the number of each
type of nuclear device in that sector.  If the sector is a nuclear
plant whose efficiency is at least 60% (i.e. a sector where you can
build nuclear warheads), then you will be given additional information
about the sector relevant to the building of nukes.
.s1
.EX nuke #1
.NF +0.3i
  sect        eff num nuke-type         lcm   hcm   oil   rad avail
  21,-3   n  100%  2 5mt fusion              499   499  1498   783   549
                  21 100kt fission
                   1 250kt fusion
  19,-3   +  100% 12 5mt fusion
                   3 50kt neutron
.FI
Note that the sector information is printed only once, and all nukes
in that sector are listed below it.
.s1
.SA "build, launch, transport, Planes"
