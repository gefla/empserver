/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  empsched.c: Show the update schedule
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2010
 */

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "optlist.h"
#include "prototypes.h"
#include "version.h"

#define DFLT_N 16
#define MAX_N 1000

static void
print_usage(char *program_name)
{
    printf("Usage: %s [OPTION]... [FILE]\n"
	   "Print the Empire update schedule.\n\n"
	   "  -e CONFIG-FILE  configuration file\n"
	   "                  (default %s)\n"
	   "  -n NUMBER       print at most NUMBER updates (default %d)\n"
	   "  -h              display this help and exit\n"
	   "  -v              display version information and exit\n\n"
	   "If FILE is given, print the schedule defined there instead of\n"
	   "the current schedule.\n",
	   program_name, dflt_econfig, DFLT_N);
}

int
main(int argc, char *argv[])
{
    char *config_file = NULL;
    char *in_file;
    unsigned long n = DFLT_N;
    time_t sched[MAX_N + 1], anchor;
    int opt, i;

    while ((opt = getopt(argc, argv, "e:n:hv")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 'n':
	    n = strtoul(optarg, NULL, 10);
	    if (n > MAX_N) {
		fprintf(stderr, "%s: can't print more than %d updates",
			argv[0], MAX_N);
		exit(1);
	    }
	    break;
	case 'h':
	    print_usage(argv[0]);
	    exit(0);
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    exit(0);
	default:
	    fprintf(stderr, "Try -h for help.\n");
	    exit(1);
	}
    }

    if (argc - optind > 1) {
	fprintf(stderr, "%s: extra operand %s\n", argv[0], argv[optind+1]);
	fprintf(stderr, "Try -h for help.\n");
	exit(1);
    }

    if (emp_config(config_file) < 0)
	exit(1);

    if (!argv[optind])
	in_file = schedulefil;
    else if (!strcmp(argv[optind], "-"))
	in_file = NULL;
    else
	in_file = argv[optind];

    anchor = (time(NULL) + 59) / 60 * 60;
    if (read_schedule(in_file, sched, n + 1, 0, anchor) < 0)
	exit(1);

    for (i = 0; sched[i]; i++)
	printf("%s", ctime(&sched[i]));

    return 0;
}
