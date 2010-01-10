/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  conftab.c: Load game configuration files
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include "file.h"
#include "optlist.h"
#include "product.h"
#include "prototypes.h"
#include "xdump.h"

static int read_custom_table_file(char *);

/*
 * Read builtin configuration tables.
 * Return 0 on success, -1 on failure.
 */
int
read_builtin_tables(void)
{
    struct empfile *ep;
    FILE *fp;
    int lineno, res;

    /*
     * Need to read config files for tables referenced through
     * ep->cadef[].ca_table before those for ep.  empfile[] is ordered
     * that way.
     */
    for (ep = empfile; ep->name; ep++) {
	if (!EF_IS_GAME_STATE(ep->uid) && ep->file) {
	    if ((fp = fopenat(ep->file, "r", builtindir)) == NULL) {
		fprintf(stderr, "Can't open %s for reading (%s)\n",
			ep->file, strerror(errno));
		return -1;
	    }
	    lineno = 1;
	    res = xundump(fp, ep->file, &lineno, ep->uid);
	    if (res >= 0 && getc(fp) != EOF) {
		fprintf(stderr, "%s: Junk after the table\n",
			ep->file);
		res = EF_BAD;
	    }
	    fclose(fp);
	    if (res < 0)
		return -1;
	}
    }

    return 0;
}

/*
 * Read user configuration tables.
 * Return 0 on success, -1 on failure.
 */
int
read_custom_tables(void)
{
    char *tmp = strdup(custom_tables);
    char *fname;
    int res = 0;

    for (fname = strtok(tmp, " \t"); fname; fname = strtok(NULL, " \t")) {
	if (read_custom_table_file(fname) < 0)
	    res = -1;
    }

    free(tmp);
    return res;
}

/*
 * Read configuration table file FNAME.
 * Return 0 on success, -1 on error.
 */
static int
read_custom_table_file(char *fname)
{
    int lineno, res, n;
    FILE *fp;

    if (!(fp = fopenat(fname, "r", configdir))) {
	fprintf(stderr, "Can't open config table %s for reading (%s)\n",
		fname, strerror(errno));
	return -1;
    }

    lineno = 1;
    for (n = 0; (res = xundump(fp, fname, &lineno, EF_BAD)) >= 0; n++)
	empfile[res].flags |= EFF_CUSTOM;
    if (res != EF_BAD && n == 0)
	fprintf(stderr, "Warning: configuration file %s is empty\n", fname);

    fclose(fp);
    return -(res == EF_BAD);
}
