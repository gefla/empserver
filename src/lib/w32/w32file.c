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
 *  w32file.c: POSIX file operations emulation layer for Windows
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#include <config.h>

#include <direct.h>
#include <io.h>
#include "unistd.h"

/*
 * POSIX-compatible replacement for mkdir().
 * Windows' mkdir() lacks the second parameter.
 */
int
w32_mkdir(const char *dirname, mode_t perm)
{
    int result;

    result = _mkdir(dirname);
    if (result < 0)
	return -1;
    return _chmod(dirname, perm);
}
