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
 *  border.c: Create a border around a map
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "map.h"
#include "optlist.h"
#include "prototypes.h"
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
	*p = 0;
	ptr += MAPWIDTH(size);
    }
}

void
border(struct range *rp, char *prefstr, char *sep)

				/* prefixes each line */
				/* separates the numbers */
{
    int posi, n, x;

    if ((WORLD_X / 2) >= 100) {
	if (rp->lx + rp->width > 100 || rp->hx - rp->width < -100) {
	    /*
	     * hundreds
	     */
	    pr("%s", prefstr);
	    for (x = rp->lx, n = 0; n < rp->width; n++, x++) {
		if (x >= WORLD_X / 2)
		    x -= WORLD_X;
		pr("%s", sep);
		if (x < 0 && x > -100) {
		    pr("-");
		} else {
		    posi = (x < 0 ? -x : x) / 100;
		    pr("%d", posi % 10);
		}
	    }
	    pr("\n");
	}
    }
    /*
     * tens
     */
    pr("%s", prefstr);
    for (x = rp->lx, n = 0; n < rp->width; n++, x++) {
	if (x >= WORLD_X / 2)
	    x -= WORLD_X;
	pr("%s", sep);
	if (x < 0 && x > -10) {
	    pr("-");
	} else {
	    posi = (x < 0 ? -x : x) / 10;
	    pr("%d", posi % 10);
	}
    }
    pr("\n");
    /*
     * units...
     */
    pr("%s", prefstr);
    for (x = rp->lx, n = 0; n < rp->width; n++, x++) {
	if (x >= WORLD_X / 2)
	    x -= WORLD_X;
	posi = (x < 0 ? -x : x);
	pr("%s%d", sep, posi % 10);
    }
    pr("\n");
}
