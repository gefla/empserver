#!/usr/local/bin/perl


#   Run by cron at 7:50 to archive (in web space) the current blitz mpeg
#   and to generate a new html index file.  Outputs html.
#	-Drake (dld@chem.psu.edu)

require 'ctime.pl';

chdir '/home/children/html/howitzer';

@file = ("Blitz-0", "Blitz-1","Blitz-2","Blitz-3","Blitz-4", "Blitz-5","Blitz-6", "Blitz-7");
unlink("$file[$#file].mpg","$file[$#file].gif","$file[$#file].txt","$file[$#file]Ann.txt");


for ($i=$#file;$i>0;$i--) {
    rename($file[$i-1] . '.mpg',"$file[$i].mpg");
    rename($file[$i-1] . '.gif',"$file[$i].gif");
    rename($file[$i-1] . '.txt',"$file[$i].txt");
    rename($file[$i-1] . 'Ann.txt',"$file[$i]Ann.txt");
}

if (-s 'Blitz.mpg') { system('cp -p Blitz.mpg Blitz-0.mpg'); }
if (-s 'Blitz.gif') { system('cp -p Blitz.gif Blitz-0.gif'); }
if (-s 'Blitz.txt') { system('cp -p Blitz.txt Blitz-0.txt'); }
if (-s 'Blitz.txt') { system('cp -p BlitzAnn.txt Blitz-0Ann.txt'); }

print "<html> <head>\n";
print "<title>Howitzer Blitz Archive</title>\n";
print "</head>\n<body>\n";
print "<h1>Archived Blitzes</h1>\n";
print "<ul>\n";

foreach $base (@file) {
    $file = $base . '.mpg';
    
    if (-s $file) {
	($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
	 $atime,$mtime,$ctime,$blksize,$blocks)
	    = stat($file);
	chop($m=&ctime($mtime));
	print "<li> $m <ul>\n";
	print "<li> <a href=\"$file\"> <img alt=\"MPEG\" src=\"$base.gif\"><br>($size bytes)</a>\n";
	print "<li> <a href=\"$base.gif\">Final Map</a>\n";
	print "<li> <a href=\"$base.txt\">Reports</a>\n\n";
	print "<li> <a href=\"$base" . "Ann.txt\">Announcements</a>\n</ul>\n";
    }
}

print "</ul>\n";
print "</body> </html>\n";
