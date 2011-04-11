/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  empdump.c: Export/import Empire game state
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2008-2010
 */

#include <config.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "file.h"
#include "optlist.h"
#include "nat.h"
#include "prototypes.h"
#include "version.h"
#include "xdump.h"

static void exit_bad_arg(char *, ...)
    ATTRIBUTE((noreturn, format (printf, 1, 2)));
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
    FILE *impf = NULL;
    int dirty[EF_MAX];

    while ((opt = getopt(argc, argv, "e:i:mnxhv")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 'i':
	    import = optarg;
	    break;
	case 'm':
	    human = 0;
	    break;
	case 'n':
	    private = EFF_PRIVATE;
	    break;
	case 'x':
	    export = 1;
	    break;
	case 'h':
	    printf("Usage: %s [OPTION]...\n"
		   "  -e CONFIG-FILE  configuration file\n"
		   "                  (default %s)\n"
		   "  -i DUMP-FILE    import from DUMP-FILE\n"
		   "  -m              use machine-readable format\n"
		   "  -n              dry run, don't update game state\n"
		   "  -x              export to standard output\n"
		   "  -h              display this help and exit\n"
		   "  -v              display version information and exit\n",
		   argv[0], dflt_econfig);
	    exit(0);
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    exit(0);
	default:
	    exit_bad_arg(NULL);
	}
    }

    if (argv[optind])
	exit_bad_arg("%s: extra operand %s\n", argv[0], argv[optind]);

    if (!import && !export)
	exit_bad_arg("%s: nothing to do!\n", argv[0]);

    if (import) {
	impf = fopen(import, "r");
	if (!impf) {
	    fprintf(stderr, "%s: Can't open %s for reading (%s)\n",
		    argv[0], import, strerror(errno));
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
	fprintf(stderr, "%s: Can't chdir to %s (%s)\n",
		argv[0], gamedir, strerror(errno));
	exit(1);
    }
    global_init();

    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i))
	    continue;
	if (!ef_open(i, EFF_MEM | private, -1))
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
	if (fclose(stdout) != 0) {
	    fprintf(stderr, "%s: error writing export (%s)\n",
		    argv[0], strerror(errno));
	    exit(1);
	}
    }

    /* write out imported data */
    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i))
	    continue;
	if (dirty[i]) {
	    if (!ef_close(i))
		exit(1);
	}
    }

    return 0;
}

static void
exit_bad_arg(char *complaint, ...)
{
    va_list ap;

    if (complaint) {
	va_start(ap, complaint);
	vfprintf(stderr, complaint, ap);
	va_end(ap);
    }
    fprintf(stderr, "Try -h for help.\n");
    exit(1);
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

    xdinit(&xd, NATID_BAD, human, printf_wrapper);
    xdhdr(&xd, ef_nameof(type), 0);
    xdcolhdr(&xd, ca);
    for (i = 0; (p = ef_ptr(type, i)); i++) {
	xdflds(&xd, ca, p);
	printf("\n");
    }
    xdftr(&xd, i);
}
