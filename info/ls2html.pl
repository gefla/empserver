#!/usr/local/bin/perl 

use strict;
use warnings;

printf("<pre>\n");
while (<>) {
    s/([\w\.\-]+)\.html/<a href=\"$1.html\">$1<\/a>/g;
    print;
}
printf("</pre>\n");
