#!/usr/local/bin/perl
#
# checklist.pl
#
# By Ken Stevens <children@empire.net>
#
# FIXME This does not work at the moment
#
# HOW TO RUN IT:
# In empire, redirect the output of the player "list" command to a file called
# "player.list".  Similarly, make a file listing deity commands and call it
# deity.list.  Put both these files in this directory, and then run this script
# to check which Empire commands need to be documented.
#

use strict;
use warnings;

my ($com, @list, @obsolete, @Commands);

open(LIST, "<player.list") || die "Can't read player.list\n";

while(<LIST>) {
  last if /^  <TYPE>/;
}

while(<LIST>) {
  last if /^For further info on command syntax see/;
  $_ = substr($_, 5);
  ($com) = split;
  push (@list, $com);
}
close LIST;

push(@list, "break");

open(LIST, "<deity.list") || die "Can't read deity.list\n";

while(<LIST>) {
  last if /^  <TYPE>/;
}

while(<LIST>) {
  last if /^For further info on command syntax see/;
  $_ = substr($_, 5);
  ($com) = split;
  push (@list, $com);
}
close LIST;

open(OBSOLETE, "<Subjects/Obsolete.t") ||
  die "Can't read Subjects/Obsolete.t\n";

while (<OBSOLETE>) {
  push(@obsolete, $1) if /^.L (\S+)$/;
}

close OBSOLETE;

open (LS, "ls Commands|");

while (<LS>) {    
  chop;
  next unless /^(\S+).t/;
  push(@Commands, $1);
}
close LS;

print "In list but not Commands:\n";
for my $l (@list) {
  print "  $l\n" unless grep (/^$l$/, @Commands);
}
print "In Commands but not list:\n";
for my $c (@Commands) {
  print "  $c\n" unless grep(/^$c$/, @list) || grep(/^$c$/, @obsolete);
}
