#!/usr/local/bin/perl

use strict;
use warnings;

my $dome = 0;
my $esc="\\";
my @a;

line: while (<>) {
    chomp;			# strip record separator
    s/([^\\](\\\\)*)\\\".*/$1/g;# strip comments

    @a = req($_);

    if (!@a) {
	if ($dome) {
	    while ($_ =~ /[A-Za-z0-9\-\.]+/g) {
		print htmlify("$`");
		print anchor("$&");
		$_="$'";
	    }
	}
	print htmlify($_), "\n";
	next line;
    }

    # requests

    if ($a[1] eq "TH") {
	@a = checkarg(2, @a);
	$a[3] = htmlify($a[3]);
	print "<title>$a[2] : $a[3]</title><h1>$a[2] : $a[3]</h1>\n";
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

    if ($a[1] eq "L") {
	$a[2] =~ / /;
	print "<br>" . anchor("$`") . " $'\n";
	next line;
    }

    if ($a[1] =~ "eo") { $esc = 0; next line; }
    if ($a[1] =~ "ec") { $esc = $#a == 1 ? "\\" : $a[2]; next line; }

    if (/^\.(NF|nf)/) { $dome = 1; printf (("<p><pre>\n")); next line; }
    if (/^\.(FI|fi)/) { $dome = 0; printf (("</pre><p>\n")); next line; }
    if (/^\.s3/) { printf (("<p>\n"));	next line; }
    if (/^\.s1/) { printf (("<hr> \n")); next line; }
    if (/^\.br/) { printf "<br>\n"; next line; }

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

sub req {
    local ($_) = @_;
    if (/^([\.\'])[ \t]*([^ ]+) *(.*)/) {
	my @a = ($1, $2);
	$_ = $3;
	while (/(\"((\\.|[^\\\"])*)(\"|$))|(([^ ]|\\.)+) */g) {
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
    # FIXME don't create dangling links here
    return "<a href=\"$_.html\">$_</a>";
}

# Translate HTML special characters into escape sequences
sub htmlify {
        local ($_) = @_;
	die "funny escape character `$esc' not supported"
	    if $esc && $esc ne "\\";
	# translate some troff escapes
	s/\\&//g if $esc;	# zero width space character
	# escape HTML special characters
        s/\&/&amp;/g;
        s/\</&lt;/g;
        s/\>/&gt;/g;
	return $_ unless $esc;
	# translate more troff escapes
	s/\\e/&\#92;/g;		# escape character
	# turn quoted strings that look like info names into links
	# tacky...
	while (/(\\\*Q)([A-Za-z0-9\-\.]+)(\\\*U)/) {
	    $_ = $` . anchor($2) . "$'";
	}
	while (/(\"info )([A-Za-z0-9\-\.]+)/) {
	    $_ = "$`\"info " . anchor($2) . "$'";
	}
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
	s/\\\*\(iF/<em>/g;	# italic font
	s/\\\(mu/x/g;		# multiply symbol
	s/\\ /&nbsp;/g;		# non breaking space
        return $_;
}
