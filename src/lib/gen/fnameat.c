/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fnameat.c: Interpret file names relative to a directory
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2008
 */

#include <config.h>

#include <errno.h>
#include "prototypes.h"

static int fname_is_abs(const char *);

/*
 * Interpret FNAME relative to directory DIR.
 * Return FNAME if it is absolute, or DIR is null or empty.
 * Else return a malloc'ed string containing DIR/FNAME, or null
 * pointer when that fails.
 */
char *
fnameat(const char *fname, const char *dir)
{
    char *res;

    if (fname_is_abs(fname) || !dir || !*dir)
	return (char *)fname;

    res = malloc(strlen(dir) + 1 + strlen(fname) + 1);
    if (!res)
	return NULL;

    sprintf(res, "%s/%s", dir, fname);
    return res;
}

static int
fname_is_abs(const char *fname)
{
#ifdef _WIN32
    /* Treat as absolute if it starts with '/', '\\' or a drive */
    return fname[0] == '/' || fname[0] == '\\'
	|| (fname[0] != 0 && fname[1] == ':');
#else
    return fname[0] == '/';
#endif
}

/*
 * Open a stream like fopen(), optionally relative to a directory.
 * This is exactly like fopen(), except FNAME is interpreted relative
 * to DIR if that is neither null nor empty.
 */
FILE *
fopenat(const char *fname, const char *mode, const char *dir)
{
    char *fnat;
    FILE *fp;

    fnat = fnameat(fname, dir);
    if (!fnat)
	return NULL;

    fp = fopen(fnat, mode);

    if (fnat != fname)
	free(fnat);

    return fp;
}
