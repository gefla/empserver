/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  bp.c: The `build pointer' (bp) map
 * 
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 *     Markus Armbruster, 2007
 */

/*
 * Some commands call update functions to simulate the update.
 * Naturally, these functions must not change game state then.
 * Instead, they track values separate data structure, namely the bp
 * map.
 */

#include <config.h>

#include "budg.h"
#include "update.h"

/* Item types we want to track.  */
enum bp_item_idx {
    BP_NONE = -1,		/* not tracked */
    BP_MILIT, BP_LCM, BP_HCM,
    BP_MAX = BP_HCM
};

/*
 * Stuff to track for a sector.
 * A bp map is an array of these.
 */
struct bp {
    short bp_item[BP_MAX + 1];
    short bp_avail;
};

/* Map i_type to enum bp_item_idx.  */
static enum bp_item_idx bud_key[I_MAX + 1] = {
    BP_NONE, BP_MILIT, BP_NONE, BP_NONE,
    BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE,
    BP_LCM, BP_HCM, BP_NONE, BP_NONE
};

/* Return pointer to the element of BP belonging to SP.  */
static struct bp *
bp_ref(struct bp *bp, struct sctstr *sp)
{
    return &bp[sp->sct_x + sp->sct_y * WORLD_X];
}

/*
 * Return the item value tracked in BP for sector SP's item COMM.
 * COMM must be a tracked item type.
 */
int
bp_get_item(struct bp *bp, struct sctstr *sp, i_type comm)
{
    enum bp_item_idx idx = bud_key[comm];

    if (CANT_HAPPEN(idx < 0))
	return sp->sct_item[comm];
    return bp_ref(bp, sp)->bp_item[idx];
}

/* Set the item value tracked in BP for sector SP's item COMM.  */
void
bp_put_item(struct bp *bp, struct sctstr *sp, i_type comm, int amount)
{
    enum bp_item_idx idx = bud_key[comm];

    if (idx >= 0)
	bp_ref(bp, sp)->bp_item[idx] = amount;
}

/* Set the item values tracked in BP for sector SP from VEC.  */
void
bp_put_items(struct bp *bp, struct sctstr *sp, short *vec)
{
    enum bp_item_idx idx;
    struct bp *p = bp_ref(bp, sp);
    i_type i;

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	idx = bud_key[i];
	if (idx >= 0)
	    p->bp_item[idx] = vec[i];
    }
}

/* Return avail tracked in BP for sector SP.  */
int
bp_get_avail(struct bp *bp, struct sctstr *sp)
{
    return bp_ref(bp, sp)->bp_avail;
}

/* Set avail tracked in BP for sector SP.  */
void
bp_put_avail(struct bp *bp, struct sctstr *sp, int amount)
{
    bp_ref(bp, sp)->bp_avail = amount;
}

/* Set the values tracked in BP for sector SP to the values in SP.  */
void
bp_set_from_sect(struct bp *bp, struct sctstr *sp)
{
    bp_put_items(bp, sp, sp->sct_item);
    bp_put_avail(bp, sp, sp->sct_avail);
}

/*
 * Return a new bp map.
 * Caller should pass it to free() when done with it.
 */
struct bp *
bp_alloc(void)
{
    return calloc(WORLD_X * WORLD_Y, sizeof(struct bp));
}
