#!/usr/local/bin/perl

use strict;
use warnings;

my (@Fld, $str, @a);

line: while (<>) {
    chop;	# strip record separator
    @Fld = split(' ', $_, 9999);

procline:
    if (/^\.TH/) {
	$str=$Fld[2];
	for (my $i=3; $i <= $#Fld; $i++) {
	    $str .= " " . $Fld[$i];
	}
	$str = &htmlify($str);
	printf("<title>%s : %s</title><h1>%s : %s</h1>\n",
	       $Fld[1], $str, $Fld[1], $str);
	next line;
    }

    if (/^\.SY../) {
#	$i = $_ =~ "\"" && ($RLENGTH = length($&), $RSTART = length($`)+1);
#	$str = substr($_, $i + 1, length($_) - $i - 1);
	$str = substr($_,5);
	$str = &htmlify($str);
	printf "<samp>[##:##] </samp><KBD>%s</KBD><p>\n", $str;
	next line;
    }

    if (/^\.EX../) {
	$str = substr($_, 5);
	printf "<br><samp>[##:##] </samp><kbd>%s</kbd><p>\n", &htmlify($str);
	next line;
    }

    if (/^\.L../) {
	$str = substr($_, 4);
	printf "<h2>%s</h2>\n", &htmlify($str);
	next line;
    }

    if (/^\.(NF|nf)/) {	printf (("<p><pre>\n")); next line;    }
    if (/^\.(FI|fi)/) {	printf (("</pre><p>\n")); next line;   }
    if (/^\.s3/) { printf (("<p>\n"));	next line; }
    if (/^\.s1/) {printf (("<hr> \n")); next line; }
    if (/^\.br/) { printf "<br>\n"; next line; }
    if (/^\.SA/) {
	@a = split('[: ",.]+');

	printf("See also : %s\n",&anchor($a[2]) );
	for (my $i = 3; $i <= $#a ; ($i)++) {       
	    printf(", %s\n",&anchor($a[$i]));
	}

	while (<>) {
	    chop;	# strip record separator
	    @a = split('[: ,.]+');
	    @Fld = split(' ', $_, 9999);
	    if (/^\./) { goto procline; }
	    for (my $i = 0; $i <= $#a ; ($i)++) {       
		printf(", %s\n",&anchor($a[$i]));
	    }
	}
	    
	exit(0);
    }
	    
    if (/^\./) {	next line; }

    if (/^(See also|See Also|see also)/) {
	@a = split('[: ,.]+');

	printf("See also : %s\n",&anchor($a[2]) );
	for (my $i = 3; $i <= $#a ; ($i)++) {       
	    printf(", %s\n",&anchor($a[$i]));
	}

	while (<>) {
	    chop;	# strip record separator
	    @a = split('[: ,.]+');
	    @Fld = split(' ', $_, 9999);
	    if (/^\./) { goto procline; }
	    for (my $i = 0; $i <= $#a ; ($i)++) {       
		printf(", %s\n",&anchor($a[$i]));
	    }
	}
	    
	exit(0);
    }
	    
    printf "%s\n", &htmlify($_);
}

#sub anchor {
#    local($_) = @_;
#    $file = $_ . ".t";
#    if (-r $file) {
#	return ("<a href=" . $_ . ".html" . ">" . $_ . "</a>");
#    } else {
#	return ( " " . $_ . " ");
#    }
#}

sub anchor {
    local ($_) = @_;
    my (@file,$file);
    $file = $_ . ".t";
#    if (-r $file) {
    if (1) {
      $file =~ s/.t$/.html/;
      return ("<a href=\"$file\">$_</a>");
    } else {
	@file = <$_*t>;
	if (@file) {
	  warn "Expanding $_ to $file[0]\n";
	  $file[0] =~ s/.t$/.html/;
	  return ("<a href=\"$file[0]\">$_</a>");
	} else {
	  warn "Unable to link $_\n";
	  return ( "<em>$_</em>");
	}
    }
}

    
# Translate HTML special characters into escape sequences
sub htmlify {
        local ($_) = @_;
	s/^\"(.*)\"$/$1/;
	s/\\&//g;		# a nothing character
        s/\&/&amp;/g;
        s/\</&lt;/g;
        s/\>/&gt;/g;
	while (@a = /(\\\*Q)([A-Za-z0-9\-\.]+)(\\\*U)/) {
	    /(\\\*Q)([A-Za-z\-]+)(\\\*U)/;
	    $_ = $` . &anchor($a[1]) . $';
	}
	while (@a = /(\\\*Q)(\"info )([A-Za-z0-9\-\.]+)(\\\*U)/) {
	    /(\\\*Q)(\"info )([\w\-\.]+)(\\\*U)/;
	    $_ = $` . "\"info " . &anchor($a[2]) . $';
	}
	while (@a = /(\"info )([A-Za-z0-9\-\.]+)/) {
	    /(\"info )([\w\-\.]+)/;
	    $_ = $` . "\"info " . &anchor($a[1]) . $';
	}
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
	s/\\ / /g;		# should be non breaking space
        return $_;
}

#sub htmlify {
#    local($str) = @_;
#    $str;
#}


