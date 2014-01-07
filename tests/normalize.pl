#!/usr/bin/perl

# TODO Don't hardcode xdump columns, get them from xdump meta

use warnings;
use strict;
use Getopt::Std;

$Getopt::Std::STANDARD_HELP_VERSION = 1;
our ($opt_j, $opt_s);
getopts('js')
    or die "$0: invalid options\n";
die "$0: either -j or -s, not both\n"
    if $opt_j && $opt_s;

my $ctime_re = qr/(Sun|Mon|Tue|Wed|Thu|Fri|Sat) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [ 123][0-9] [0-2][0-9]:[0-5][0-9]:[0-6][0-9] [0-9][0-9][0-9][0-9]/;
my $xdfld_re = qr/\([^)]*\)|[^ (][^ ]*/;

# Current dump, if any
# Either zero or the name of the dump we're currently processing.
# Legacy dump names start with an uppercase letter, and xdump names
# start with a lowercase letter.
my $dump = "";

sub norm_ctime {
    my ($s) = @_;
    $s =~ s/$ctime_re/Thu Jan  1 00:00:00 1970/g;
    return $s;
}

while (<>) {
    chomp;

    # Strip log timestamp
    if ($opt_j || $opt_s) {
	die "$0: malformed line" unless /^$ctime_re /;
	$_ = substr($_, 25);
    }

    # Strip trailing white space
    # We don't really care for it in test output, and git's pre-commit
    # hook flags it, which is annoying.
    s/\s+$//;

    # Split off prefix that is not to be normalized
    my $pfx = '';
    if ($opt_j) {
	die "$0: malformed line" unless substr($_, 10, 1) eq ' ';
	$pfx .= substr($_, 0, 11);
	# Normalize only player output
	$_ = substr($_, 11);
	if (/(^output [^ ]* 1 )(.*)/) {
	    $pfx .= $1;
	    $_ = $2;
	} else {
	    $pfx .= $_;
	    $_ = '';
	}
    }

    if ($opt_s) {
	$_ = norm_ctime($_);
	### Host environment in logs
	# getrusage() results in server.log
	s/(End update|done assembling paths) .* user .* system/$1 0.0 user 0.0 system/g;
	# PID in server.log
	s/(Empire server \(pid) [0-9]+(\) started)/$1 42$2/g;
	### Harmless races
	# shutdown wins race with logout
	next if /Waiting for player threads to terminate/;
	print "$pfx$_\n";
	next;
    }

    $dump = ""
	if ($dump =~ /^[a-z]/
	    and /^\//)
	or ($dump =~ /^[A-Z]/
	    and (/\: No (sector|ship|plane|unit|nuke)\(s\)|\: Nothing lost/
		 or /^[0-9]+ (sector|ship|plane|unit|nuke|lost item)/));

    ### Formatted time
    # nat_timeused in prompt
    s/^\[[0-9]+(:[0-9]+\] Command \:)/[0$1/g;
    # TODO command play column time
    # result of ctime() in many commands
    $_ = norm_ctime($_)
	unless $dump;
    ### Time values in legacy dumps
    s/(DUMP (SECTOR|SHIPS|PLANES|LAND UNITS|NUKES|LOST ITEMS)) [0-9]+$/$1 0/g;
    s/ [0-9]+$/ 0/
	if $dump eq 'LOST ITEMS';
    ### Time values in xdump
    s/(XDUMP (meta )?[-a-z0-9]+) [0-9]+$/$1 0/
	unless $dump;
    # HACK: assume any integer with more than 10 digits is time
    # TODO don't do that, use xdump meta instead
    s/(^| )[0-9]{10,}/${1}0/g
	if $dump =~ /^[a-z]/;
    # timeused in xdump country timeused (column 10)
    s/^(($xdfld_re ){10})([0-9]+) /${1}255 /
	if $dump eq 'country';
    # timeused in xdump nat (column 15)
    s/^(($xdfld_re ){15})([0-9]+) /${1}255 /
	if $dump eq 'nat';
    # duration in xdump news (column 4)
    s/^(($xdfld_re ){4})([0-9]+) /${1}0 /
	if $dump eq 'news';
    ### nsc_type values in xdump
    # Can vary between systems, because the width of enumeration types
    # is implementation-defined.
    # TODO type in xdump meta
    ### nrndx values in xdump
    # Encoding depends on the host, see resources[].  Too clever by half;
    # perhaps we should change it.
    # nrndx in xdump product (column 12)
    s/^(($xdfld_re ){12})([0-9]+) /${1}0 /
	if $dump eq 'product';
    # value in xdump resources (column 0)
    s/^[0-9]+ /0 /
	if $dump eq 'resources';
    ### Floating-point zero in xdump
    # Windows %#g prints it with seven significant digits instead of six
    s/ 0\.000000/ 0.00000/g
	if $dump =~ /^[a-z]/;

    print "$pfx$_\n";

    if (/(XDUMP|^config) (meta )?([-a-z0-9]+)/) {
	$dump = $3;
	die unless $dump =~ /^[a-z]/;
    } elsif (/DUMP (SECTOR|SHIPS|PLANES|LAND UNITS|NUKES|LOST ITEMS) /) {
	$dump = $1;
    }
}
