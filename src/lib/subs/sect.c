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
#include "path.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

static int checksect(struct sctstr *);
static void update_railway(struct sctstr *, struct sctstr *);

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

    bridge_damaged(sp);
    checksect(sp);
    getsect(sp->sct_x, sp->sct_y, &sect);
    if (opt_RAILWAYS)
	update_railway(sp, &sect);
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

/* Minimal efficiency for railway and railway extension (opt_RAILWAYS) */
#define SCT_RAIL_EFF 5
#define SCT_RAIL_EXT_EFF 60

/* Is sector SP a railway? */
#define SCT_IS_RAILWAY(sp) \
    (dchr[(sp)->sct_type].d_mob1 == 0 && (sp)->sct_effic >= SCT_RAIL_EFF)
/* May sector SP have a railway extension? */
#define SCT_MAY_HAVE_RAIL_EXT(sp) \
    ((sp)->sct_effic >= SCT_RAIL_EXT_EFF)
/* Does railway sector SP extend railway track into sector TOSP? */
#define SCT_EXTENDS_RAIL(sp, tosp) \
    ((sp)->sct_own == (tosp)->sct_own && SCT_MAY_HAVE_RAIL_EXT(tosp))

static void
update_railway(struct sctstr *sp, struct sctstr *oldsp)
{
    struct sctstr sect;
    int was_railway = SCT_IS_RAILWAY(oldsp);
    int is_railway = SCT_IS_RAILWAY(sp);
    int i;

    if (was_railway == is_railway
	&& sp->sct_own == oldsp->sct_own
	&& SCT_MAY_HAVE_RAIL_EXT(sp) == SCT_MAY_HAVE_RAIL_EXT(oldsp))
	return;

    if (was_railway)
	sp->sct_track--;
    if (is_railway)
	sp->sct_track++;

    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	getsect(sp->sct_x + diroff[i][0],
		sp->sct_y + diroff[i][1],
		&sect);
	if (SCT_IS_RAILWAY(&sect) && SCT_EXTENDS_RAIL(&sect, oldsp))
	    sp->sct_track--;
	if (SCT_IS_RAILWAY(&sect) && SCT_EXTENDS_RAIL(&sect, sp))
	    sp->sct_track++;
	if (was_railway && SCT_EXTENDS_RAIL(oldsp, &sect))
	    sect.sct_track--;
	if (is_railway && SCT_EXTENDS_RAIL(sp, &sect))
	    sect.sct_track++;
	putsect(&sect);
    }
}

void
set_railway(struct sctstr *sp)
{
    int i;
    struct sctstr *nsp;

    sp->sct_track = !!SCT_IS_RAILWAY(sp);
    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	nsp = getsectp(sp->sct_x + diroff[i][0],
		       sp->sct_y + diroff[i][1]);
	if (SCT_IS_RAILWAY(nsp) && SCT_EXTENDS_RAIL(nsp, sp))
	    sp->sct_track++;
    }
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
