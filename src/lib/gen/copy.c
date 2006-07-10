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
 *  copy.c: Copy and return pointer to end of copied string.
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "map.h"
#include "misc.h"
#include "optlist.h"
#include "xy.h"

/*
 * space-fill a map or radar scan;
 * null terminate
 */
void
blankfill(char *ptr, struct range *range, int size)
{
    char *p;
    int row;
    int col;

    for (row = 0; row < range->height; row++) {
	col = (range->width + 1) * (size + 1) / 2 - 1;
	p = ptr;
	while (--col >= 0)
	    *p++ = ' ';
	*p++ = 0;
	ptr += MAPWIDTH(size);
    }
}
