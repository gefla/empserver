#!/usr/local/bin/perl
#
#                                 info.pl
#        
#               Create Subjects/Subject.t files from the Info Pages.
#
#                           written by Ken Stevens
#
#
# DESCRIPTION:
# info.pl reads all of the info pages and creates a table of contents
# for them organized by subject.  An info page belongs to a subject if
# that subject appears as an entry in the .SA ("SEE ALSO") field of the
# info page _and_ that entry is not the name of another info page.
# 
# For example, the .SA field of headlines.t contains the entries
# "newspaper" and "Communication".  Since there's already an info page
# called "newspaper.t", but there is no "Communication" info page, then
# the headlines info page is considered to be a member of the
# Communication subject.
#
# An info page may belong to more than one subject, and if it belongs
# to no subject, then its subject will be set to the name of the subdirectory
# it is in (e.g. the Server and Information info pages work this way).
# 
# The output of this script is a bunch of files in the "Subjects"
# subdirectory.  The file Subjects/TOP.t is the toplevel table of
# contents and lists all of the subjects.  This is what the player
# sees when they type "info".  Then for each subject, a
# Subjects/Subject.t file is created, listing all of the info pages that
# belong to that subject.
#
# INSTALLATION:
# info.pl requires perl5 to run.  If you don't have version 5 of perl, then
# you'll either have to install it, or you'll have to get someone to create
# your Subjects.t files for you.
#
# HOW TO RUN IT:
# Type "info.pl" at the unix prompt.
#
# BUG REPORTS:
# mail your bug-reports and comments to:
# Ken Stevens <children@empire.net>

#       --- Glossary ---
# item.t          An info page file
# item            An info page
# Subject         An entry in a SEE ALSO entry which is not an item
# subdirectory    Where the info files are kept
#
#       --- Global variables ---
# @dirs           Subdirectories of info directory containing item.t files
# @Subjects       Subjects which already exist (as Subjects/Subject.t)
# $dir            The current subdirectory we're working in
# $filename       The name of an item.t file
# $filedir{$filename}
#                 The subdirectory item.t is in
# F               Filehandle for item.t
# $desc{$filename}
#                 A one line description of the item (second arg to .NA)
# $level{$filename}
#                 The difficulty level of the page.  At present either
#                 Basic or Expert.
# $see_also{$filename}
#                 A list of SEE ALSO items for the file (.SA argument)
# $subj           A subject
# SUBJ            Filehandle for Subject.t
# $subject{$subj}{$dir} = "item1\nitem2\n..."
#                 Items in that subject organized by directory.
# $largest{$sub}  The largest item in that subject (used for column formatting)
# TOP             Filehandle for Subjects/TOP.t
# @rowsubj        List of subjects
# @colsubj        List of subjects organized into 3 columns
#
#     --- Functions ---
#
# read_subjects   Get list of current subjects
# parse_files     Parse all the item.t files in one $dir
# parse_file      Check the .TH, .NA, and .SA fields & parse them
# parse_see_also  Create %subject from %see_also
# set_subject     Add a new entry to %subject and possibly to %largest
# create_subj     Create a Subject.t file
# create_subjects Remove the old Subject.t files and create new ones
# flush_subj      Print a row of Subjects to TOP
# error           Print an integrity error to STDERR and exit with code 1.

eval("require 5");		# Test for perl version 5
die "info.pl requires version 5 of perl.\n" if $@;

# These are the directories which contain item.t files:
@dirs = ('Commands', 'Concepts', 'Server', 'Introduction');

# Get list of current subjects
&read_subjects;

# Parse the item.t files in each info directory
for $dir (@dirs) {
  &parse_files;
}

# Create %subject from %see_also
for $filename (sort keys %filedir) {
  &parse_see_also;
}

# Create the Subject.t files
&create_subjects;

exit 0;

# Get list of current subjects
sub read_subjects {
  open (LS, "ls Subjects|");

  while (<LS>) {    
    chop;
    next unless /^(\S+).t/;
    push(@Subjects, $1);
  }
  close LS;
}

# Parse all the item.t files in one $dir with lots of integrity checks
sub parse_files {
  local ($type) = $dir;
  chop($type) unless $type eq "Server" || $type eq "Introduction";

  if (defined $filedir{$dir}) {
    $filename = $dir;
    &error("Illegal filename (it is a directory name).");
  } elsif (defined $filedir{$type}) {
    $filename = $type;
    &error("Illegal filename (it is a type name).");
  }

  open (LS, "ls $dir|");

  while (<LS>) {    
    chop;
    next if /^GNUmakefile$/;
    next if /^Makefile$/;
    next if /^MakeSrcs$/;
    $filename = $_;
    &parse_file;
  }
  close LS;
}

# Check the .TH, .NA, and .SA fields.
# Parse .NA into %desc and .SA into %see_also
sub parse_file {
  $filename =~ s/\.t$//;

  if (grep (/^$filename$/, @dirs)) {
    &error("Illegal filename.  $filename is a name of a subdirectory of the info directory.");
  }
  if ($filedir{$filename}) {
    &error("$filename.t is in both $filedir{$filename} and $dir");
  } elsif (grep (/^$filename$/, @Subjects)) {
    &error("Illegal filename.  $filename is already a Subject name.");
  } else {
    $filedir{$filename} = $dir;
  }

  die "Can't open $dir/$filename.t\n" unless open(F, "<$dir/$filename.t");
  
  $_ = <F>;
  if (/^\.TH (\S+) (\S.+\S)$/) {
    if ($1 ne $type) {
      &error("First argument to .TH was '$1' but it should be '$type'");
    }
    if ($type eq "Command" && $2 ne "\U$filename") {
      &error("Second argument to .TH was '$2' but it should be '\U$filename'");
    }
  } else {
    &error("The first line in the file must be a .TH entry");
  }
  $_ = <F>;
  if (/^\.NA (\S+) "(\S.+\S)"$/) {
    if ($filename ne $1) {
      &error("First argument to .NA was '$1' but it should be '$filename'");
    }
    $desc{$filename} = $2;
  } else {
    &error("The second line in the file must be an .NA entry");
  }
  $_ = <F>;
  if (/^\.LV (\S+)$/) {
    if ($1 ne 'Basic' && $1 ne 'Expert') {
      &error("The argument to .LV was '$1' but it must be either 'Basic' or 'Expert'");
    }
    $level{$filename} = $1;
  } else {
    &error("The third line in the file must be a .LV entry");
  }
  while (<F>) {
    last if /^\.SA/;
  }
  if ($_) {
    if (/^\.SA "([^\"]*)"/) {
      $see_also{$filename} = $1;
    } else {
      &error("Incorrect .SA Syntax.  Syntax should be '.SA \"item1, item2\"'");
    }
    while (<F>) {
      &error("Multiple .SA entries.  Each file may contain at most one .SA entry") if /^\.SA/;
    }
  }
  close F;
}

# Create %subject from %see_also
sub parse_see_also {
  local (@see_also) = split(/, /, $see_also{$filename});
  local ($dir) = $filedir{$filename};
  local ($found);		# Does this item belong to any Subject?

  for (@see_also) {
    if (!(defined $filedir{$_})) { # is this entry a subject?
      &set_subject;
      $found = 1;
    }
  }

  &error("No Subject listed in .SA field") unless $found;
}

# Add a new entry to %subject and possibly to %largest
sub set_subject {
  $subject{$_}{$dir} .= "$filename\n";
  $largest{$_} = $filename if length $filename > length $largest{$_};
  $largest{$_} = $dir if length $dir > length $largest{$_};
}

# Create a Subject.t file
sub create_subj {
  print "  Creating Subjects/$subj.t\n";
  print "WARNING: $subj is a NEW subject\n" unless
    grep(/^$subj$/, @Subjects);
  die "Unable to write to Subjects/$subj.t\n" unless
    open(SUBJ, ">Subjects/$subj.t");

  print SUBJ '.\" DO NOT EDIT THIS FILE.  It was automatically generated by info.pl'."\n";
  print SUBJ ".TH Subject \U$subj\n";
  $largest{$subj} =~ s/-/M/g;
  print SUBJ ".in \\w'$largest{$subj}XX\\0\\0\\0\\0'u\n";
  for $dir (keys %{$subject{$subj}}) {
    print SUBJ ".s1\n";
    for (split(/\n/, $subject{$subj}{$dir})) {
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
  print "  Removing Subjects/*.t\n";
  `rm -f Subjects/*.t`;
  print "  Creating Subjects/TOP.t\n";
  die "Can't open Subjects/TOP.t" unless open(TOP, ">Subjects/TOP.t");
  print TOP <<EOF;
.TH Info "List of Subjects"
.s1
Empire info is available on the following subjects:
.NF
EOF

  @rowsubj = sort keys %subject;

  for $subj (@Subjects) {
    print "WARNING: The subject $subj has been removed.\n" unless
      $subj eq 'TOP' || grep (/^$subj$/, @rowsubj);
  }

  for $i (0..2) {
    for ($j = $i; $j <= $#rowsubj; $j += 3) {
      $colsubj[$j] = $rowsubj[$k++];
    }
  }

  for $subj (@colsubj) {
    &create_subj;
    push(@subj, $subj);
    &flush_subj if $#subj > 1;
  }
  &flush_subj;
  print TOP <<EOF;
.FI
Type "info <Subject>" where <Subject> is one of the subjects listed above.
For a complete list of all info topics, type "info all".
EOF
  close TOP;
}

# Print a row of subjects to TOP
sub flush_subj {
  return unless $#subj >= 0;
  print TOP "  ";
  for (@subj) {
    printf TOP "%-25s", $_;
  }
  print TOP "\n";
  @subj = ();
}

# Print an integrity error message and exit with code 1
sub error {
  local ($error) = @_;

  print STDERR "Error on line $. of $filedir{$filename}/$filename.t:\n";
  print STDERR "$_";
  print STDERR "\n" unless /\n$/;
  print STDERR "$error\n";
  exit 1;
}
