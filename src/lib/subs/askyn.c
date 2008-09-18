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
 *  askyn.c: The oldest routines in the book... ask questions of the user
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1993
 *     Doug Hay, 1998
 */

#include <config.h>

#include "prototypes.h"

/*
 * If they don't answer 'y' or 'Y', return 0
 */
int
confirm(char *promptstring)
{
    char y_or_n[1024];
    char c;

    if (getstring(promptstring, y_or_n) == 0)
	return 0;
    c = *y_or_n;
    if (c == 'y' || c == 'Y')
	return 1;
    return 0;
}

/*
 * Make them answer 'y', 'Y', 'n', or 'N', and
 * return 1 for y or Y, and 0 for n or N
 */
int
askyn(char *promptstring)
{
    char y_or_n[1024];
    char c;

    while (getstring(promptstring, y_or_n)) {
	c = *y_or_n;
	if (c == 'y' || c == 'Y')
	    return 1;
	if (c == 'n' || c == 'N')
	    return 0;
	pr("You must answer either yes or no!\n");
    }
    /* assume no if aborted */
    return 0;
}
