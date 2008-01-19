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
 *  sect.c: Sector pre-write and post-read data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include <config.h>

#include <ctype.h>
#include "file.h"
#include "land.h"
#include "lost.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

static int checksect(struct sctstr *);

int
sct_postread(int id, void *ptr)
{
    struct sctstr *sp = ptr;

    checksect(sp);
    player->owner = (player->god || sp->sct_own == player->cnum);
    if (opt_MOB_ACCESS)
	sct_do_upd_mob(sp);
    return 1;
}

int
sct_prewrite(int id, void *ptr)
{
    struct sctstr *sp = ptr;
    struct sctstr sect;

    time(&sp->sct_timestamp);

    checksect(sp);
    getsect(sp->sct_x, sp->sct_y, &sect);
    return 1;
}

void
item_prewrite(short *item)
{
    i_type i;

    for (i = I_NONE + 1; i <= I_MAX; ++i) {
	if (CANT_HAPPEN(item[i] < 0))
	    item[i] = 0;
	else if (CANT_HAPPEN(item[i] > ITEM_MAX))
	    item[i] = ITEM_MAX;
    }
}

static int
checksect(struct sctstr *sp)
{
    int mil, civs, loyalcivs;

    item_prewrite(sp->sct_item);

    /* shouldn't happen, but... */
    if (sp->sct_mobil > 127)
	sp->sct_mobil = 0;

    mil = sp->sct_item[I_MILIT];
    civs = sp->sct_item[I_CIVIL];
    if (sp->sct_own == sp->sct_oldown)
	loyalcivs = civs;
    else
	loyalcivs = 0;

    if (sp->sct_effic < SCT_MINEFF) {
	if (sp->sct_type == SCT_BSPAN)
	    knockdown(sp, 0);
	else if (sp->sct_type == SCT_BTOWER) {
	    knockdown(sp, 0);
	    bridgefall(sp, 0);
	}
    }
    if (sp->sct_own != 0 && !civs) {
	sp->sct_work = 100;
	sp->sct_oldown = sp->sct_own;
    }
    /* If they have a military unit there, they still own it */
    if (sp->sct_own && !loyalcivs && !(sp->sct_flags & MOVE_IN_PROGRESS)) {
	if (!mil && !has_units(sp->sct_x, sp->sct_y, sp->sct_own, 0)) {
	    /* more cruft! */
	    if (sp->sct_type == SCT_CAPIT || sp->sct_type == SCT_MOUNT)
		caploss(sp, sp->sct_own, "");

	    if (sp->sct_oldown == sp->sct_own) {
		makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
		sp->sct_own = 0;
		sp->sct_oldown = 0;
	    } else
		takeover(sp, sp->sct_oldown);
	    sp->sct_mobil = 0;
	}
    }
    return 1;
}

int
issector(char *arg)
{
    char c;

    while (0 != (c = *arg++))
	if (!isdigit(c) && !isspace(c) && (c != '/'))
	    return 1;

    return 0;
}
