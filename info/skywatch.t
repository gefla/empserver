.TH Command SKYWATCH
.NA skywatch "Check from sectors for nearby satellites"
.LV Expert
.SY "skywatch [<SECTS>]"
The skywatch command allows sectors to report sightings of satellites.
.s1
The visibility range is equal to
four times the sector efficiency
expressed as a decimal times the technology factor;
e.g. a 100% efficient mine with a 50% technology factor can
see 2 sectors away.
An exception is radar installations, which have a maximum range
equal to fourteen times the sector efficiency, (i.e. 3.5 times as far).
.s1
A typical usage might be:
.EX sky #3 ?type=)
.s1
.SA "coastwatch, radar, Detection"
