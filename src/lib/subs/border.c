/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  border.c: Create a border around a map
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "xy.h"
#include "prototypes.h"
#include "optlist.h"

void
border(struct range *rp, s_char *prefstr, s_char *sep)

				/* prefixes each line */
				/* separates the numbers */
{
    register int posi;
    register int n;
    register int x;

    if ((WORLD_X / 2) >= 100) {
	if (rp->lx + rp->width > 99 || rp->hx - rp->width < -99) {
	    /*
	     * hundreds
	     */
	    pr(prefstr);
	    for (x = rp->lx, n = 0; n < rp->width; n++, x++) {
		if (x >= WORLD_X / 2)
		    x -= WORLD_X;
		pr(sep);
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
    pr(prefstr);
    for (x = rp->lx, n = 0; n < rp->width; n++, x++) {
	if (x >= WORLD_X / 2)
	    x -= WORLD_X;
	pr(sep);
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
    pr(prefstr);
    for (x = rp->lx, n = 0; n < rp->width; n++, x++) {
	if (x >= WORLD_X / 2)
	    x -= WORLD_X;
	posi = (x < 0 ? -x : x);
	pr("%s%d", sep, posi % 10);
    }
    pr("\n");
}
