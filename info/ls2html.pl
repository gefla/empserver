#!/usr/local/bin/perl 

printf("<pre>\n");
while (<>) {
    s/([\w\.\-]+)\.html/<a href=\"$1.html\">$1<\/a>/g;
    print;
}
printf("</pre>\n");
