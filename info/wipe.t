.TH Command WIPE
.NA wipe "Remove all distribution thresholds"
.LV Expert
.SY "wipe <SECTS>"
The wipe command removes all distribution thresholds
from the specified sectors.
Note that this will have drastic effects on sectors
that rely on distribution in any way, so use with caution.
Also note that if you make a mistake,
it can cost lots of BTU's to reverse the effects of a wipe.
.s1
.EX level #5
.NF
Wed Aug 16 18:34:43 1995
DISTRIBUTION LEVELS
  sect      dst    civ mil  uw food  sh gun pet iron dust bar oil lcm hcm rad
 -5,5   k  -1,5      0   0   0    0   0   0   0  566    1   0   0   0   1   0
 -3,5   f  -1,5      0   0   0    0 100  10   0    0    0   0   0 100 250   0
 -1,5   w  -1,5      0   0   0    0   0   0   0    0    0   0   0   0   0   0
 -6,6   !  -1,5      0   0   0    0 200  50 200    0    0   0   1 200 200   0
    4 sectors
.FI
.EX wipe #5
.NF
distribution thresholds wiped from -5,5
distribution thresholds wiped from -3,5
distribution thresholds wiped from -1,5
distribution thresholds wiped from -6,6
.FI
.EX level #5
.NF
Wed Aug 16 18:34:43 1995
DISTRIBUTION LEVELS
  sect      dst    civ mil  uw food  sh gun pet iron dust bar oil lcm hcm rad
 -5,5   k  -1,5      0   0   0    0   0   0   0    0    0   0   0   0   0   0
 -3,5   f  -1,5      0   0   0    0   0   0   0    0    0   0   0   0   0   0
 -1,5   w  -1,5      0   0   0    0   0   0   0    0    0   0   0   0   0   0
 -6,6   !  -1,5      0   0   0    0   0   0   0    0    0   0   0   0   0   0
    4 sectors
.FI
.s1
.SA "distribute, census, commodity, level, threshold, Distribution"
