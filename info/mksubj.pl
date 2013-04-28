#!/usr/bin/perl
#
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#   mksubj.pl: Create the subject index pages
#
#   Known contributors to this file:
#      Ken Stevens (when it was still info.pl)
#      Markus Armbruster, 2006-2013
#
# Usage: mksubj.pl SUBJECT... INFO-FILE...
#
# Read the INFO-FILE..., create info/SUBJECT.t for each SUBJECT.

use strict;
use warnings;
use File::stat;

use Fcntl qw(O_WRONLY O_EXCL O_CREAT);

# The chapters, in order
my @Chapters = qw/Introduction Concept Command Server/;

my @Levels = qw/Basic Expert Obsolete/;
my %Subjects;

# $filename{TOPIC} is TOPIC's file name
my %filename;
# $long{TOPIC} is true when TOPIC's page is "long"
my %long;
# $chapter{TOPIC} is TOPIC's chapter (first arg to .TH)
my %chapter;
# $desc{TOPIC} is a one line description of TOPIC (second arg to .NA)
my %desc;
# $level{TOPIC} is TOPIC's difficulty level (arg to .LV)
my %level;
# $see_also{TOPIC} is TOPIC's list of SEE ALSO items (.SA argument)
my %see_also;
# $sanr{TOPIC} is the line number of TOPIC's .SA request
my %sanr;

# current info file
my $filename;

# $subject{$subj}{$chap} = "item1\nitem2\n..."
#                 Topics in that subject organized by chapter.
my %subject;
# $largest{$sub}  The largest topic name in that subject (used for
#                 column formatting)
my %largest;

while ($#ARGV >= 0 && $ARGV[0] !~ /\.t$/) {
    $Subjects{shift @ARGV} = undef;
}

for (@ARGV) {
    parse_file($_);
}

for my $t (sort keys %desc) {
    parse_see_also($t);
}

create_subjects();

# Parse an info file
# Set $filename, $filename{TOPIC}, $long{TOPIC}, $chapter{TOPIC},
# $desc{TOPIC}, $level{TOPIC}, $see_also{TOPIC}, $sanr{TOPIC}
sub parse_file {
    ($filename) = @_;
    my ($topic, $st);

    $topic = $filename;
    $topic =~ s,.*/([^/]*)\.t$,$1,;
    $filename{$topic} = $filename;

    $st = stat $filename
	or die "Can't stat $filename: $!";
    $long{$topic} = $st->size > 9999;

    open(F, "<$filename")
	or die "Can't open $filename: $!";

    $_ = <F>;
    if (/^\.TH (\S+) (\S.+\S)$/) {
	if (!grep(/^$1$/, @Chapters)) {
	    error("First argument to .TH was '$1', which is not a known chapter");
	}
	$chapter{$topic} = $1;
	if ($1 eq "Command" && $2 ne "\U$topic") {
	    error("Second argument to .TH was '$2' but it should be '\U$topic'");
	}
    } else {
	error("The first line in the file must be a .TH request");
    }

    $_ = <F>;
    if (/^\.NA (\S+) "(\S.+\S)"$/) {
	if ($topic ne $1) {
	    error("First argument to .NA was '$1' but it should be '$topic'");
	}
	$desc{$topic} = $2;
    } else {
	error("The second line in the file must be a .NA request");
    }

    $_ = <F>;
    if (/^\.LV (\S+)$/) {
	if (!grep(/^$1$/, @Levels)) {
	    error("The argument to .LV was '$1', which is not a known level");
	}
	$level{$topic} = $1;
    } else {
	error("The third line in the file must be a .LV request");
    }

    while (<F>) {
	last if /^\.SA/;
    }

    if ($_) {
	if (/^\.SA "([^\"]*)"/) {
	    $see_also{$topic} = $1;
	    $sanr{$topic} = $.;
	} else {
	    error("Incorrect .SA Syntax.  Syntax should be '.SA \"item1, item2\"'");
	}

	while (<F>) {
	    error("Multiple .SA requests.  Each file may contain at most one.") if /^\.SA/;
	}
    } else {
	error(".SA request is missing");
    }

    close F;
}

# Create %subject and %largest from %see_also
sub parse_see_also {
    my ($topic) = @_;
    my @see_also = split(/, /, $see_also{$topic});
    my $wanted = $chapter{$topic};
    my $found;		       # found a subject?

    $wanted = undef if $wanted eq 'Concept' or $wanted eq 'Command';
    $filename = $filename{$topic};
    $. = $sanr{$topic};

    for (@see_also) {
	if (!exists $desc{$_}) { # is this entry a subject?
	    error("Unknown topic $_ in .SA") unless exists $Subjects{$_};
	    set_subject($_, $topic);
	    $found = 1;
	}
	if ($wanted && $_ eq $wanted) {
	    $wanted = undef;
	}
    }

    error("No subject listed in .SA") unless $found;
    error("Chapter $wanted not listed in .SA") if $wanted;
}

# Add a new entry to %subject and possibly to %largest
sub set_subject {
    my ($sub, $topic) = @_;
    my $chap = $chapter{$topic};
    $subject{$sub}{$chap} .= "$topic\n";
    $largest{$sub} = "" unless defined $largest{$_};
    $largest{$sub} = $topic if length $topic > length $largest{$sub};
}

# Create a Subject.t file
sub create_subj {
    my ($subj) = @_;
    my $fname = "info/$subj.t";
    my ($any_topic, $any_basic, $any_obsolete, $any_long);

    sysopen(SUBJ, $fname, O_WRONLY | O_EXCL | O_CREAT)
	or die "Unable to create $fname: $!\n";

    print SUBJ '.\" DO NOT EDIT THIS FILE.  It was automatically generated by mksubj.pl'."\n";
    print SUBJ ".TH Subject \U$subj\n";
    $largest{$subj} =~ s/-/M/g;
    print SUBJ ".in \\w'$largest{$subj}XX\\0\\0\\0\\0'u\n";
    for my $chap (@Chapters) {
	next unless exists $subject{$subj}{$chap};
	print SUBJ ".s1\n";
	for my $topic (split(/\n/, $subject{$subj}{$chap})) {
	    $any_topic = 1;
	    my $flags = "";
	    if ($level{$topic} eq 'Basic') {
		$flags .= "*";
		$any_basic = 1;
	    }
	    if ($level{$topic} eq 'Obsolete') {
		$flags .= "+";
		$any_obsolete = 1;
	    }
	    if ($long{$topic}) {
		$flags .= "!";
		$any_long = 1;
	    }
	    $flags = sprintf("%-2s", $flags);
	    print SUBJ ".L \"$topic $flags\"\n";
	    print SUBJ "$desc{$topic}\n";
	}
    }
    unless ($any_topic) {
	print STDERR "$0: Subject $subj has no topics\n";
	exit 1;
    }
    print SUBJ ".s1\n"
	. ".in 0\n"
	. "For info on a particular subject, type \"info <subject>\" where <subject> is\n"
	. "one of the subjects listed above.\n";
    print SUBJ "Subjects marked by * are the most important and should be read by new players.\n"
	if $any_basic;
    print SUBJ "Subjects marked by + are obsolete.\n"
	if $any_obsolete;
    print SUBJ "Unusually long subjects are marked with a !.\n"
	if $any_long;
    close SUBJ;
}

# Remove the old Subject.t files and create new ones
sub create_subjects {
    my (@subj);

    for (keys %Subjects) {
	unlink "info/$_.t";
    }

    @subj = sort keys %subject;

    for my $subj (@subj) {
	create_subj($subj);
    }
}

# Print an integrity error message and exit with code 1
sub error {
    my ($error) = @_;

    print STDERR "mksubj.pl:$filename:$.: $error\n";
    exit 1;
}
