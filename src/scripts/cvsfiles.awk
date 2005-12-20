#!/usr/bin/awk -f
# Find files under CVS control
# FIXME Ignores CVS/Entries.log

BEGIN {
    FS="/";
    if (ARGC == 1)
	ARGV[ARGC++] = "CVS/Entries";
    else {
	for (i = 1; i < ARGC; i++)
	    ARGV[i] = ARGV[i] "/CVS/Entries";
    }
}

{ cwd = FILENAME; sub(/CVS\/Entries$/, "", cwd); }

/^\// {
    if ($3 !~ /^-/)
	print cwd $2;
}

/^D/ && NF > 1 {
    ARGV[ARGC++] = cwd $2 "/CVS/Entries";
}
