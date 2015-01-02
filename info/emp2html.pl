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
#   emp2html.pl: Convert info source to HTML
#
#   Known contributors to this file:
#      Drake Diedrich, 1996
#      Markus Armbruster, 2004-2013
#
# Usage: emp2html.pl INFO...
#
# Convert info source on standard input to HTML on standard output.
# INFO... are the info page names.

use strict;
use warnings;

my $in_nf = 0;
my $esc = "\\";
my $ignore = 0;
my $is_subj;
my @a;
my %topic;

for (@ARGV) {
    $topic{$_} = 1;
}

print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n";
print "   \"http://www.w3.org/TR/html4/strict.dtd\">\n";
print "<html>\n";
print "<head>\n";

line: while (<STDIN>) {
    chomp;			# strip record separator
    s/((^|[^\\])(\\\\)*)\\\".*/$1/g; # strip comments

    @a = req($_);

    if (!@a) {
	if ($is_subj && $in_nf) {
	    while ($_ =~ /[A-Za-z0-9\-\.]+/g) {
		print htmlify("$`");
		print anchor("$&");
		$_="$'";
	    }
	}
	print htmlify($_), "\n" unless $ignore;
	next line;
    }

    # requests

    if ($a[1] eq "ig") { $ignore = 1; next line; }
    if ($ignore) {
	$ignore = 0 if $a[1] eq "..";
	next line;
    }

    if ($a[1] eq "TH") {
	@a = checkarg(2, @a);
	$is_subj = $a[2] eq 'Subject' || $a[2] eq 'Info';
	$a[3] = htmlify($a[3]);
	print "<title>$a[2] : $a[3]</title>\n";
	print "</head>\n";
	print "<body>\n";
	print "<h1>$a[2] : $a[3]</h1>\n";
	print "<p>\n";
	next line;
    }

    if ($a[1] eq "SY") {
	@a = checkarg(1, @a);
	$a[2] = htmlify($a[2]);
	print "<samp>[##:##] Command : </samp><KBD>$a[2]</KBD><p>\n";
	next line;
    }

    if ($a[1] eq "EX") {
	my $str = htmlify(join(' ',@a[2..$#a]));
	print "<br><samp>[##:##] Command : </samp><kbd>$str</kbd><p>\n";
	next line;
    }

    if ($a[1] eq "L" && $is_subj) {
	$a[2] =~ / /;
	print "<br>" . anchor("$`") . " $'\n";
	next line;
    }

    if ($a[1] =~ /^LV?$/) {
	@a = checkarg(1, @a);
	$a[2] = htmlify($a[2]);
	print "<h2>$a[2]</h2>\n";
	print "<p>\n";
	next line;
    }

    if ($a[1] eq "eo") { $esc = 0; next line; }
    if ($a[1] eq "ec") { $esc = $#a == 1 ? "\\" : $a[2]; next line; }

    if ($a[1] =~ /NF|nf/i) { $in_nf = 1; printf (("<p><pre>\n")); next line; }
    if ($a[1] =~ /FI|fi/i) { $in_nf = 0; printf (("</pre><p>\n")); next line; }
    if ($a[1] eq "s1") { printf (("<hr><p>\n")); next line; }
    if ($a[1] eq "br") { printf "<br>\n"; next line; }

    if ($a[1] eq "SA") {
	@a = checkarg(1, @a);
	@a = split(/[\: \"\,\.]+/, $a[2]);
	for my $a (@a) {
	    $a = anchor($a);
	}
	print "<p>See also : ", join("\n, ", @a), "\n";
    }

    # ignore unknown request
}

print "</body>\n";
print "</html>\n";

sub req {
    local ($_) = @_;
    if (/^([\.\'])[ \t]*([^ ]*) *(.*)/) {
	my @a = ($1, $2);
	$_ = $3;
	while (/\G(\"((\\.|[^\\\"])*)(\"|\Z))|\G(([^ ]|\\.)+) */g) {
	    push(@a, $2 || $5);
	}
	return @a;
    }
    return ();
}

sub checkarg {
    my ($n, @a) = @_;
    warn "extra arguments for $a[1] ignored" if $#a > $n+1;
    warn "missing arguments for $a[1] supplied" if $#a < $n+1;
    while ($#a < $n+1) {
	push @a, "";
    }
    return @a;
}

sub anchor {
    local ($_) = @_;
    return $topic{$_} ? "<a href=\"$_.html\">$_</a>" : $_;
}

# Translate HTML special characters into escape sequences
sub htmlify {
    local ($_) = @_;
    die "funny escape character `$esc' not supported"
	if $esc && $esc ne "\\";
    # translate some troff escapes
    s/\\&//g if $esc;		# zero width space character
    # escape HTML special characters
    s/\&/&amp;/g;
    s/\</&lt;/g;
    s/\>/&gt;/g;
    return $_ unless $esc;
    # translate more troff escapes
    s/\\e/&\#92;/g;		# escape character
    # turn quoted strings that look like info names into links
    # tacky...
    my $pfx = "";
    while (/\\\*Q([A-Za-z0-9\-\.]+)\\\*U|\"info ([A-Za-z0-9\-\.]+)\"/) {
	if (defined $1 && $topic{$1}) {
	    $pfx = $` . anchor($1);
	} elsif (defined $2 && $topic{$2}) {
	    $pfx = "$`\"info " . anchor($2) . "\"";
	} else {
	    $pfx .= $` . $&;
	}
	$_ = "$'";
    }
    $_ = "$pfx$_";
    # tranlate more troff escapes and strings
    s/\\\*Q/<em>/g;
    s/\\\*U/<\/em>/g;
    s/\\fI/<em>/g;
    s/\\fR/<\/em><\/em>/g;
    s/\\fB/<strong>/g;
    s/\\fP/<\/strong><\/em>/g;
    s/\\\*\(bF/<strong>/g;	# bold font
    s/\\\*\(pF/<\/strong><\/em>/g; # pica font
    s/\\\*\(nF/<\/strong><\/em>/g; # normal font
    s/\\\*\(iF/<em>/g;		# italic font
    s/\\\(mu/x/g;		# multiply symbol
    s/\\ /&nbsp;/g;		# non breaking space
    return $_;
}
