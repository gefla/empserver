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
 *  bp.c: Functions for build pointer (bp) handling
 * 
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 */

#include "misc.h"
#include "nat.h"
#include "sect.h"
#include "budg.h"
#include "update.h"
#include "common.h"
#include "optlist.h"

static int bud_key[I_MAX + 2] =
    { 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 5, 6, 0, 0, 7 };

int *
get_wp(int *bp, struct sctstr *sp, int cm)
{
    return (bp + (sp->sct_x + (sp->sct_y * WORLD_X)) +
	    WORLD_X * WORLD_Y * (cm - 1));
}

int
gt_bg_nmbr(int *bp, struct sctstr *sp, i_type comm)
{
    int *wp;
    int cm;

    if ((cm = bud_key[comm]) == 0) {
	return sp->sct_item[comm];
    } else {
	wp = get_wp(bp, sp, cm);
	return *wp;
    }
}

void
pt_bg_nmbr(int *bp, struct sctstr *sp, i_type comm, int amount)
{
    int *wp;
    int cm;

    if ((cm = bud_key[comm]) != 0) {
	wp = get_wp(bp, sp, cm);
	*wp = amount;
    }
}

void
fill_update_array(int *bp, struct sctstr *sp)
{
    int k;
    int *wp;
    i_type i;

    for (i = I_NONE + 1; i <= I_MAX; i++)
	if ((k = bud_key[i]) != 0) {
	    wp = get_wp(bp, sp, k);
	    *wp = sp->sct_item[i];
	}
    wp = get_wp(bp, sp, bud_key[I_MAX + 1]);
    *wp = sp->sct_avail;
}
