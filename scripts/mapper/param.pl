#!/usr/local/bin/perl

open(FRAMES_FH, 'frames') || die 'Cannot open file "frames".';


# modify the INPUT stanza of a mpeg param file to reflect the actual input

$[ = 1;			# set array base to 1
$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

line: while (<>) {
    chop;	# strip record separator
    @Fld = split(' ', $_, 9999);
    if (/^INPUT$/ .. /^END_INPUT$/) {
	next line;
    }

    print $_;
}

printf (("INPUT\n"));
$_ = &Getline2('FRAMES_FH');
$frames = $Fld[1];
for ($i = 0; $i <= $frames; $i++) {
    $file = sprintf('map-%d.gif', $i);
    $size = -s $file;
    if ($size>300 && $size<10000) {
	printf "%s\n", $file;
    }
}
printf (("END_INPUT\n"));

sub Getline2 {
    ($fh) = @_;
    if ($getline_ok = (($_ = <$fh>) ne '')) {
	chop;	# strip record separator
	@Fld = split(' ', $_, 9999);
    }
    $_;
}
