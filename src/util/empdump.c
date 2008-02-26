/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  empdump.c: Export/import Empire game state
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2008
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"
#include "version.h"
#include "xdump.h"

static void dump_table(int, int);

int
main(int argc, char *argv[])
{
    char *config_file = NULL;
    char *import = NULL;
    int export = 0;
    int private = 0;
    int human = 1;
    int opt, i, lineno, type;
    FILE *impf;
    int dirty[EF_MAX];

    while ((opt = getopt(argc, argv, "e:mtxhv")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 'h':
	    printf("Usage: %s [OPTION]... [DUMP-FILE]\n"
		   "  -e CONFIG-FILE  configuration file\n"
		   "                  (default %s)\n"
		   "  -m              use machine-readable format\n"
		   "  -t              test import, don't update game state\n"
		   "  -x              export to standard output\n"
		   "  -h              display this help and exit\n"
		   "  -v              display version information and exit\n",
		   argv[0], dflt_econfig);
	    exit(0);
	case 'm':
	    human = 0;
	    break;
	case 't':
	    private = EFF_PRIVATE;
	    break;
	case 'x':
	    export = 1;
	    break;
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    exit(0);
	default:
	    fprintf(stderr, "Try -h for help.\n");
	    exit(1);
	}
    }

    if (argv[optind])
	import = argv[optind++];

    if (import) {
	impf = fopen(import, "r");
	if (!impf) {
	    fprintf(stderr, "Cant open %s for reading (%s)\n",
		    import, strerror(errno));
	    exit(1);
	}
    } else
	private = EFF_PRIVATE;

    /* read configuration & initialize */
    empfile_init();
    if (emp_config(config_file) < 0)
	exit(1);
    empfile_fixup();
    nsc_init();
    if (read_builtin_tables() < 0)
	exit(1);
    if (read_custom_tables() < 0)
	exit(1);
    if (chdir(gamedir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n",
		gamedir, strerror(errno));
	exit(1);
    }
    global_init();

    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i))
	    continue;
	if (!ef_open(i, EFF_MEM | private))
	    exit(1);
    }

    /* import from IMPORT */
    memset(dirty, 0, sizeof(dirty));
    if (import) {
	lineno = 1;
	while ((type = xundump(impf, import, &lineno, EF_BAD)) >= 0)
	    dirty[type] = 1;
	if (type == EF_BAD)
	    exit(1);
    }

    if (ef_verify() < 0)
	exit(1);

    /* export to stdout */
    if (export) {
	for (i = 0; i < EF_MAX; i++) {
	    if (!EF_IS_GAME_STATE(i))
		continue;
	    dump_table(i, human);
	}
    }

    /* write out imported data */
    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i))
	    continue;
	if (!private && dirty[i]) {
	    if (!ef_close(i))
		exit(1);
	}
    }

    return 0;
}

static void
printf_wrapper(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

static void
dump_table(int type, int human)
{
    struct xdstr xd;
    struct castr *ca;
    int i;
    void *p;

    ca = ef_cadef(type);
    if (!ca)
	return;

    xdinit(&xd, 0, human, printf_wrapper);
    xdhdr(&xd, ef_nameof(type), 0);
    xdcolhdr(&xd, ca);
    for (i = 0; (p = ef_ptr(type, i)); i++) {
	xdflds(&xd, ca, p);
	printf("\n");
    }
    xdftr(&xd, i);
}
