.TH Command SECT
.NA sect "Show only sectors owned by you in map form"
.LV Expert
.SY "sect <SECTS>"
The sect report gives you a graphic representation of all or part
of your country, similar to map.
Only sectors owned by you show up, however.
The rest of the world is simply blank.
.s1
If your terminal is of the proper type,
any sectors selected by the \*Q?cond\*U operator
will show up in reverse video.
.s1
Examples:
.EX sect -9:18,-8:5
generates a 28 x 14 map based on data supplied by the sectors
in the area specified.
.NF
    ---------0000000000111111111
    9876543210123456789012345678
 -8                              -8
 -7                              -7
 -6                              -6
 -5           a a                -5
 -4          k o !               -4
 -3       a h j a                -3
 -2          a a a   a a         -2
 -1         a a a b + +          -1
  0        m c u a a             0
  1         a a   a a            1
  2          a a a   a           2
  3                              3
  4                              4
  5                              5
    ---------0000000000111111111
    9876543210123456789012345678
.FI
.s1
.s1
.SA "census, commodity, radar, realm, route, map, Maps"
