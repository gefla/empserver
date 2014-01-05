#!/usr/bin/perl

use warnings;
use strict;

my $toc = shift @ARGV;
my %cmd;
my $status = 0;

open TOC, "<$toc"
    or die "Can't open $toc for reading: $!";
while (<TOC>) {
    my ($chap, $top) = split;
    $cmd{$top} = undef if $chap eq 'Command';
}
close TOC;

while (<>) {
    next unless (/^[ 0-9][0-9] [$ ][c ] (\S+)/);
    gripe("$1 lacks an info page\n")
	unless exists $cmd{$1};
    $cmd{$1} = 1;
}

for (keys %cmd) {
    gripe("$_ is not in any command list\n")
	unless defined $cmd{$_}
}

sub gripe {
    print STDERR @_;
    $status = 1;
}

exit $status;
