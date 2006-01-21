#!/usr/bin/perl
#
#  Empire - A multi-player, client/server Internet based war game.
#  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
#                           Ken Stevens, Steve McClure
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  ---
#
#  See files README, COPYING and CREDITS in the root of the source
#  tree for related information and legal notices.  It is expected
#  that future projects/authors will amend these files as needed.
#
#  ---
#
#  mktop.pl: Create the index of subjects
#
#  Known contributors to this file:
#     Ken Stevens (when it was still info.pl)
#     Markus Armbruster, 2006
#

# Usage: mktop.pl OUTFILE SUBJECT-INDEX-FILE...
# The SUBJECT-INDEX-FILE... contain all the subject indexes.  Derive
# the subjects from the file names, write the index to OUTFILE.  Only
# the file names are used, the files aren't accessed.

my $out = shift @ARGV;
my @subject = ();
for (@ARGV) {
    /([^\/]*)\.t$/
	or die "Strange subject file name";
    push @subject, $1;
}
@subject = sort @subject;

open(TOP, ">$out")
    or die "Can't open $out: $!";
print TOP <<EOF;
.TH Info "List of Subjects"
.s1
Empire info is available on the following subjects:
.NF
EOF

# reorder subjects for display in three columns
my $k = 0;
for my $i (0..2) {
    for (my $j = $i; $j <= $#subject; $j += 3) {
	$colsubj[$j] = $subject[$k++];
    }
}

for my $subj (@colsubj) {
    push(@subj, $subj);
    if ($#subj > 1) {
	flush_subj(@subj);
	@subj = ();
    }
}
flush_subj(@subj);
print TOP <<EOF;
.FI
Type "info <Subject>" where <Subject> is one of the subjects listed above.
For a complete list of all info topics, type "info all".
EOF
close TOP;

# Print a row of subjects to TOP
sub flush_subj {
    return unless $#_ >= 0;
    print TOP "  ";
    for (@_) {
	printf TOP "%-25s", $_;
    }
    print TOP "\n";
}
