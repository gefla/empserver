#!/usr/bin/perl
#
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
#                 Ken Stevens, Steve McClure, Markus Armbruster
#
#   Empire is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   ---
#
#   See files README, COPYING and CREDITS in the root of the source
#   tree for related information and legal notices.  It is expected
#   that future projects/authors will amend these files as needed.
#
#   ---
#
#   ls-sources.pl: List source files
#
#   Known contributors to this file:
#      Markus Armbruster, 2015
#
# Usage: ls-sources.pl DIR REGEX
#
# List source files in DIR with names matching REGEX.  If DIR is under
# version control, list version-controlled files.  Else, list files in
# sources.mk.

use warnings;
use strict;

if ($#ARGV != 1) {
    print STDERR "Usage: $0 DIR REGEX\n";
    exit 1;
}

my $dir = $ARGV[0];
my $regex = $ARGV[1];

my $srcdir = $dir;
my $subdir = "";

while (! -d "$srcdir/.git" and ! -r "$srcdir/sources.mk"
       and $srcdir =~ m,^(.*)/([^/]*),) {
    $srcdir = $1;
    $subdir = "$2/$subdir";
}

my @sources;
if (-d "$srcdir/.git") {
    open IN, "cd '$srcdir' && git ls-files '$subdir' | uniq |"
	or die "can't run git ls-files: $!";
    while (<IN>) {
	chomp;
	push @sources, $_;
    }
    close IN;
} elsif (-r "$srcdir/sources.mk") {
    open IN, "<$srcdir/sources.mk"
	or die "can't open $srcdir/sources.mk: $!";
    $_ = <IN>;
    s/.*=//;
    @sources = grep m,^\Q$subdir\E,, split;
    close IN;
} else {
    print STDERR "Can't find source tree\n";
    exit 1;
}

for (grep m/$regex/, @sources) {
    print "$srcdir/$_\n";
}
