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
 *  material.c: Tries to find materials for production
 * 
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 */

#include "misc.h"
#include "sect.h"
#include "budg.h"
#include "player.h"
#include "update.h"
#include "common.h"
#include "subs.h"

#ifndef MIN
#define MIN(x,y)        ((x) > (y) ? (y) : (x))
#endif

void
get_materials(struct sctstr *sp, int *bp, int *mvec, int check)
	       /* only check if found=0, remove them=1 */
{
    i_type i;
    int still_left;

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	if (mvec[i] == 0)
	    continue;

	if (check) {
	    still_left = gt_bg_nmbr(bp, sp, i);
	    if ((still_left - mvec[i]) < 0)
		still_left = 0;
	    else
		still_left -= mvec[i];
	    pt_bg_nmbr(bp, sp, i, still_left);
	    if (!player->simulation)
		sp->sct_item[i] = still_left;

	} else {
	    still_left = gt_bg_nmbr(bp, sp, i);
	    mvec[i] = MIN(mvec[i], still_left);
	}
    }
}
