#!/usr/local/bin/perl

use strict;
use warnings;

printf("<pre>\n");
while (<>) {
    s/([\w\.\-]+)/<a href=\"$1.html\">$1<\/a>/g;
    print;
}
printf("</pre>\n");
