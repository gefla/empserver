#
#	Riverworld Blitz mpeg makefile.  Assumes the netpbm programs are
#	somewhere in the path.  map.m2v is an mpeg2 file-- unfortunately
#	it's 15x the size of the mpeg1 (.mpg) file.  If you modify map.par
#	to generate a smaller file, or find another mpeg2 encoder, let
#	me know.
#		-Drake (dld@chem.psu.edu)


.SUFFIXES: .pgm .gif .ppm .pov .jpg

all: map.mpg final.gif

map.param: param.pl default.param frames
	param.pl default.param >map.param
color.ppm: randppm.awk
	randppm.awk >$@
map.mpg: map.param 
	mpeg_encode map.param
final.gif: frames color.ppm
	makeframe map-`cat frames`.gif | ppmtogif -interlace >final.gif

clean:
	rm -f map-*.gif overmap-*.gif frames census report version over.pbm power wire final.gif over.mask color.ppm map.mpg
