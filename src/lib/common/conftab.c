/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2006
 *  
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "prototypes.h"
#include "optlist.h"
#include "file.h"

static int read_config_table_file(char *);

/*
 * Read user configuration tables.
 * Return 0 on success, -1 on failure.
 */
int
read_config_tables(void)
{
    char *tmp = strdup(config_tables);
    char *fname;
    int res = 0;

    for (fname = strtok(tmp, " \t"); fname; fname = strtok(NULL, " \t")) {
	if (read_config_table_file(fname) < 0)
	    res = -1;
    }

    free(tmp);
    return res;
}

/*
 * Read configuaration table file FNAME.
 * Return 0 on success, -1 on error.
 */
static int
read_config_table_file(char *fname)
{
    int res, n;
    FILE *fp;

    if (!(fp = fopen(fname, "r"))) {
	fprintf(stderr, "Can't open config table %s for reading (%s)\n",
		fname, strerror(errno));
	return -1;
    }

    for (n = 0; (res = xundump(fp, fname, EF_BAD)) >= 0; n++) ;
    if (res != EF_BAD && n == 0)
	fprintf(stderr, "Warning: configuration file %s is empty\n", fname);

    fclose(fp);
    return -(res == EF_BAD);
}
