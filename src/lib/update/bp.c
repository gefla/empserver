/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#include "empobj.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "update.h"
#include "xy.h"

/* Item types we want to track. */
enum bp_item_idx {
    BP_NONE = -1,		/* not tracked */
    BP_MILIT, BP_LCM, BP_HCM,
    BP_MAX = BP_HCM
};

enum bp_status {
    BP_UNUSED,			/* not tracked, values are invalid */
    BP_WANTED,			/* tracked, values are still invalid */
    BP_USED			/* tracked, values are valid */
};

/*
 * Stuff to track for a sector.
 * A bp map is an array of these.
 */
struct bp {
    short bp_item[BP_MAX + 1];
    short bp_avail;
    unsigned char bp_status;
};

/* Map i_type to enum bp_item_idx. */
static enum bp_item_idx bud_key[I_MAX + 1] = {
    BP_NONE, BP_MILIT, BP_NONE, BP_NONE,
    BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE,
    BP_LCM, BP_HCM, BP_NONE, BP_NONE
};

/* Return true when @bp doesn't track @sp. */
int
bp_skip_sect(struct bp *bp, struct sctstr *sp)
{
    return bp && bp[sp->sct_uid].bp_status == BP_UNUSED;
}

/* Return true when @bp doesn't track @unit's sector. */
int
bp_skip_unit(struct bp *bp, struct empobj *unit)
{
    return bp && bp[XYOFFSET(unit->x, unit->y)].bp_status == BP_UNUSED;
}

/* If @unit belongs to the player, start tracking its sector in @bp. */
void
bp_consider_unit(struct bp *bp, struct empobj *unit)
{
    int id;

    if (!bp || unit->own != player->cnum)
	return;
    id = XYOFFSET(unit->x, unit->y);
    if (bp[id].bp_status == BP_UNUSED)
	bp[id].bp_status = BP_WANTED;
}

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
    bp[sp->sct_uid].bp_status = BP_USED;
}

/*
 * Copy the values tracked in @bp for sector @sp back to it.
 * Values must have been set with bp_set_from_sect().
 */
void
bp_to_sect(struct bp *bp, struct sctstr *sp)
{
    i_type i;
    enum bp_item_idx idx;

    if (CANT_HAPPEN(bp[sp->sct_uid].bp_status != BP_USED))
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
 * The map initially tracks the sectors belonging to the player.
 */
struct bp *
bp_alloc(void)
{
    int n = WORLD_SZ();
    struct bp *bp = malloc(n * sizeof(*bp));
    int i;

    for (i = 0; i < n; i++)
	bp[i].bp_status = getsectid(i)->sct_own == player->cnum
	    ? BP_WANTED : BP_UNUSED;

    return bp;
}
