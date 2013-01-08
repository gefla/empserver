/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fsize.c: BSD dependant file and block sizing routines
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 */

#include <config.h>

#include <sys/stat.h>
#include <unistd.h>
#include "prototypes.h"

/*
 * return the size of the file in bytes.
 */
int
fsize(int fd)
{
    struct stat statb;

    if (fstat(fd, &statb) < 0)
	return -1;
    return statb.st_size;
}

/*
 * Return the preferred block size for I/O on FD.
 */
int
blksize(int fd)
{
#if defined(_WIN32)
    return 2048;
#else  /* !_WIN32 */
    struct stat statb;

    if (fstat(fd, &statb) < 0)
	return 1024;
    return statb.st_blksize;
#endif /* !_WIN32 */
}

time_t
fdate(int fd)
{
    struct stat statb;

    if (fstat(fd, &statb) < 0)
	return 0;
    return statb.st_mtime;
}
