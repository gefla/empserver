#!/usr/local/bin/perl
#
#                                 info.pl
#        
#                 Create SUBJECT.t files from the Info Pages.
#
#                           written by Ken Stevens
#
#
# DESCRIPTION:
# info.pl reads all of the info pages and creates a table of contents
# for them organized by subject. 
#
# Info consists of pages organized into chapters and subjects.  Each
# page is about a topic.  The page for topic ITEM is in info file
# info/ITEM.t.  An info page's chapter is the first argument of its
# .TH request.  It belongs to a subject if that subject appears in its
# .SA request ("SEE ALSO") _and_ that entry is not the name of another
# info page.  An info page may belong to more than one subject.
#
# For example, the .SA request of headlines.t contains "newspaper" and
# "Communication".  Since there's already an info page called
# "newspaper.t", but there is no "Communication" info page, the
# headlines info page is considered to be a member of the
# Communication subject.
#
# This script reads GNUmakefile and sources.mk to find info sources.
# It reads existing subjects from subjects.mk, and updates that file.
# It creates a file info/SUBJECT.t for each SUBJECT, and a table of
# subjects info/TOP.t.
#
# REQUIREMENTS:
# info.pl requires perl5 to run.  If you don't have version 5 of perl, then
# you'll either have to install it, or you'll have to get someone to create
# your Subjects.t files for you.
#
# HOW TO RUN IT:
# Run "info.pl" at the root of the build tree.

#       --- Global variables ---
# @Subjects       Existing subjects
# @Chapters       Existing chapters
# $filename       The name of the current info file
# $chapter{TOPIC} TOPIC's chapter (first arg to .TH)
# $desc{TOPIC}    A one line description of TOPIC (second arg to .NA)
# $level{TOPIC}   TOPIC's difficulty level (arg to .LV)
# $see_also{TOPIC}
#                 TOPIC's SEE ALSO items (.SA argument)
# $sanr{TOPIC}    Line number of TOPIC's .SA request
# $subject{$subj}{$chap} = "item1\nitem2\n..."
#                 Topics in that subject organized by chapter.
# $largest{$sub}  The largest topic name in that subject (used for
#                 column formatting)
#
#     --- File handles ---
# F               Filehandle for info page sources and makefiles
# SUBJ            Filehandle for Subject.t
# TOP             Filehandle for TOP.t
#
#     --- Functions ---
#
# read_make_var   Read a variable value from a makefile
# parse_file      Check the .TH, .NA, and .SA fields & parse them
# parse_see_also  Create %subject from %see_also
# set_subject     Add a new entry to %subject and possibly to %largest
# create_subj     Create a Subject.t file
# create_subjects Remove the old Subject.t files and create new ones
# flush_subj      Print a row of Subjects to TOP
# error           Print an integrity error to STDERR and exit with code 1.

use strict;
use warnings;

use Errno qw(ENOENT);
use Fcntl qw(O_WRONLY O_EXCL O_CREAT);

our (%chapter, %desc, %level, %see_also, %sanr);
our ($filename, %subject, %largest);

eval("require 5");		# Test for perl version 5
die "$0 requires version 5 of perl.\n" if $@;

# The chapters, in order
our @Chapters = qw/Introduction Concept Command Server/;

# Get known subjects
our @Subjects = split(' ', read_make_var("subjects", "subjects.mk", ""));
# Get source directory
my $srcdir = read_make_var("srcdir", "GNUmakefile");
# Get info sources
my @tsrc = grep(/\.t$/, split(' ' , read_make_var("src", "sources.mk")));

# Parse the .t files
for my $t (@tsrc) {
    parse_file("$srcdir/$t");
}

# Create %subject from %see_also
for my $t (sort keys %desc) {
    parse_see_also($t);
}

# Create the Subject.t files
@Subjects = create_subjects();

# Update subjects.mk
open(F, ">subjects.mk")
    or die "Can't open subjects.mk for writing: $!";
print F "subjects := " . join(' ', @Subjects) . "\n";
close(F);

exit 0;

# Read a variable value from a makefile
sub read_make_var {
    my ($var, $fname, $dflt) = @_;
    my $val;

    unless (open(F, "<$fname")) {
	return $dflt if $! == ENOENT and defined $dflt;
	die "Can't open $fname: $!";
    }
    while (<F>) {
	if (/^[ \t]*\Q$var\E[ \t]:?=*(.*)/) {
	    $val = $1;
	    last;
	}
    }
    close(F);
    $val or die "Can't find $var in $fname";
    return $val;
}

# Check .TH, .NA, .LV and .SA.
# Parse .NA into %desc and .SA into %see_also
sub parse_file {
    ($filename) = @_;
    my $topic;

    $topic = $filename;
    $topic =~ s,.*/([^/]*)\.t$,$1,;
    
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
	if ($1 ne 'Basic' && $1 ne 'Expert') {
	    error("The argument to .LV was '$1' but it must be either 'Basic' or 'Expert'");
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

# Create %subject from %see_also
sub parse_see_also {
    my ($topic) = @_;
    my @see_also = split(/, /, $see_also{$topic});
    my $found;		       # found a subject?

    $filename = "$srcdir/$topic";

    for (@see_also) {
	if (!exists $desc{$_}) { # is this entry a subject?
	    set_subject($_, $topic);
	    $found = 1;
	}
    }

    $. = $sanr{$topic};
    error("No subject listed in .SA") unless $found;
}

# Add a new entry to %subject and possibly to %largest
sub set_subject {
    my ($sub, $topic) = @_;
    my $chap = $chapter{$topic};
    $subject{$sub}{$chap} .= "$topic\n";
    $largest{$sub} = "" unless defined $largest{$_};
    $largest{$sub} = $topic if length $topic > length $largest{$sub};
    $largest{$sub} = $chap if length $chap > length $largest{$_};
}

# Create a Subject.t file
sub create_subj {
    my ($subj) = @_;
    my $fname = "info/$subj.t";

    print "  Creating $fname\n";
    print "WARNING: $subj is a NEW subject\n"
	unless grep(/^$subj$/, @Subjects);
    sysopen(SUBJ, $fname, O_WRONLY | O_EXCL | O_CREAT)
	or die "Unable to create $fname: $!\n";

    print SUBJ '.\" DO NOT EDIT THIS FILE.  It was automatically generated by info.pl'."\n";
    print SUBJ ".TH Subject \U$subj\n";
    $largest{$subj} =~ s/-/M/g;
    print SUBJ ".in \\w'$largest{$subj}XX\\0\\0\\0\\0'u\n";
    for my $chap (@Chapters) {
	next unless exists $subject{$subj}{$chap};
	print SUBJ ".s1\n";
	for (split(/\n/, $subject{$subj}{$chap})) {
	    print SUBJ ".L \"$_ ";
	    if ($level{$_} eq 'Basic') {
		print SUBJ "* \"\n";
	    } else {
		print SUBJ "  \"\n";
	    }
	    print SUBJ "$desc{$_}\n";
	}
    }
    print SUBJ <<EOF;
.s1
.in 0
For info on a particular subject, type "info <subject>" where <subject> is
one of the subjects listed above.  Subjects marked by * are the most
important and should be read by new players.
EOF
    close SUBJ;
}

# Remove the old Subject.t files and create the Subject.t files and TOP.t
sub create_subjects {
    my (@colsubj, @rowsubj, @subj);

    print "  Removing Subjects\n";
    for (@Subjects) {
	unlink "info/$_.t";
    }
    print "  Creating info/TOP.t\n";
    open(TOP, ">info/TOP.t")
	or die "Can't open info/TOP.t: $!";
    print TOP <<EOF;
.TH Info "List of Subjects"
.s1
Empire info is available on the following subjects:
.NF
EOF

    @rowsubj = sort keys %subject;

    for my $subj (@Subjects) {
	print "WARNING: The subject $subj has been removed.\n"
	    unless grep (/^$subj$/, @rowsubj);
    }

    # reorder subjects for display in three columns
    my $k = 0;
    for my $i (0..2) {
	for (my $j = $i; $j <= $#rowsubj; $j += 3) {
	    $colsubj[$j] = $rowsubj[$k++];
	}
    }

    for my $subj (@colsubj) {
	create_subj($subj);
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
    return @rowsubj;
}

# Print a row of subjects to TOP
sub flush_subj {
    return unless $#_ >= 0;
    print TOP "  ";
    for (@_) {
	printf TOP "%-25s", $_;
    }
    print TOP "\n";
}

# Print an integrity error message and exit with code 1
sub error {
    my ($error) = @_;

    print STDERR "info.pl:$filename:$.: $error\n";
    exit 1;
}
