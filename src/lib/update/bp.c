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
 *  bp.c: Functions for build pointer (bp) handling
 * 
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "budg.h"
#include "update.h"

enum {
    BP_NONE = -1,
    BP_CIVIL, BP_MILIT, BP_SHELL, BP_GUN, BP_LCM, BP_HCM,
    BP_MAX = BP_HCM
};

struct bp {
    short bp_item[BP_MAX + 1];
    short bp_avail;
};

static int bud_key[I_MAX + 1] = {
    BP_CIVIL, BP_MILIT, BP_SHELL, BP_GUN,
    BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE, BP_NONE,
    BP_LCM, BP_HCM, BP_NONE, BP_NONE
};

static struct bp *
bp_ref(struct bp *bp, struct sctstr *sp)
{
    return &bp[sp->sct_x + sp->sct_y * WORLD_X];
}

int
bp_get_item(struct bp *bp, struct sctstr *sp, i_type comm)
{
    int idx = bud_key[comm];

    if (idx < 0)
	return sp->sct_item[comm];
    return bp_ref(bp, sp)->bp_item[idx];
}

void
bp_put_item(struct bp *bp, struct sctstr *sp, i_type comm, int amount)
{
    int idx = bud_key[comm];

    if (idx >= 0)
	bp_ref(bp, sp)->bp_item[idx] = amount;
}

int
bp_get_avail(struct bp *bp, struct sctstr *sp)
{
    return bp_ref(bp, sp)->bp_avail;
}

void
bp_put_avail(struct bp *bp, struct sctstr *sp, int amount)
{
    bp_ref(bp, sp)->bp_avail = amount;
}

void
bp_set_from_sect(struct bp *bp, struct sctstr *sp)
{
    int idx;
    struct bp *p = bp_ref(bp, sp);
    i_type i;

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	idx = bud_key[i];
	if (idx >= 0)
	    p->bp_item[idx] = sp->sct_item[i];
    }
    p->bp_avail = sp->sct_avail;
}

struct bp *
bp_alloc(void)
{
    return calloc(WORLD_X * WORLD_Y, sizeof(struct bp));
}
