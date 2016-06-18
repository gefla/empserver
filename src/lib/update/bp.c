/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  bp.c: The `build pointer' (bp) map
 *
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 *     Markus Armbruster, 2007-2016
 */

/*
 * Some commands call update functions to simulate the update.
 * Naturally, these functions must not change game state then.
 * Instead, they track values separate data structure, namely the bp
 * map.
 */

#include <config.h>

#include <stdlib.h>
#include "optlist.h"
#include "update.h"

/* Item types we want to track. */
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
    unsigned char bp_tracked;
};

/* Map i_type to enum bp_item_idx. */
static enum bp_item_idx bud_key[I_MAX + 1] = {
    BP_NONE, BP_MILIT, BP_NONE, BP_NONE,
    BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE,
    BP_LCM, BP_HCM, BP_NONE, BP_NONE
};

/* Set the values tracked in @bp for sector @sp to the values in @sp. */
void
bp_set_from_sect(struct bp *bp, struct sctstr *sp)
{
    i_type i;
    enum bp_item_idx idx;

    if (!bp)
	return;
    for (i = I_NONE + 1; i <= I_MAX; i++) {
	idx = bud_key[i];
	if (idx >= 0)
	    bp[sp->sct_uid].bp_item[idx] = sp->sct_item[i];
    }
    bp[sp->sct_uid].bp_avail = sp->sct_avail;
    bp[sp->sct_uid].bp_tracked = 1;
}

/*
 * Copy the values tracked in @bp for sector @sp back to @sp.
 * Values must have been set with bp_set_from_sect().
 */
void
bp_to_sect(struct bp *bp, struct sctstr *sp)
{
    i_type i;
    enum bp_item_idx idx;

    if (CANT_HAPPEN(!bp[sp->sct_uid].bp_tracked))
	return;

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	idx = bud_key[i];
	if (idx >= 0)
	    sp->sct_item[i] = bp[sp->sct_uid].bp_item[idx];
    }
    sp->sct_avail = bp[sp->sct_uid].bp_avail;
}

/*
 * Return a new bp map.
 * Caller should pass it to free() when done with it.
 */
struct bp *
bp_alloc(void)
{
    int n = WORLD_SZ();
    struct bp *bp = malloc(n * sizeof(*bp));
    int i;

    for (i = 0; i < n; i++)
	bp[i].bp_tracked = 0;
    return bp;
}
