/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sect.c: Sector pre-write and post-read data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nat.h"
#include "file.h"
#include "land.h"
#include "nsc.h"
#include "plane.h"
#include "prototypes.h"
#include "optlist.h"

static int checksect(struct sctstr *);
static void give_back_cap(struct sctstr *sp);

/*ARGSUSED*/
int
sct_postread(int id, s_char *ptr)
{
    struct sctstr *sp = (struct sctstr *)ptr;

    checksect(sp);
    player->owner = (player->god || sp->sct_own == player->cnum);
    if (opt_MOB_ACCESS)
	sct_do_upd_mob(sp);
    return 1;
}

/*ARGSUSED*/
int
sct_prewrite(int id, s_char *ptr)
{
    struct sctstr *sp = (struct sctstr *)ptr;
    struct sctstr sect;

    time(&sp->sct_timestamp);

    checksect(sp);
    getsect(sp->sct_x, sp->sct_y, &sect);
    return 1;
}

static int
checksect(struct sctstr *sp)
{
    int mil, civs, loyalcivs;

    /* shouldn't happen, but... */
    if (sp->sct_mobil > 127)
	sp->sct_mobil = 0;

    mil = getvar(V_MILIT, (s_char *)sp, EF_SECTOR);
    civs = getvar(V_CIVIL, (s_char *)sp, EF_SECTOR);
    if (sp->sct_own == sp->sct_oldown)
	loyalcivs = civs;
    else
	loyalcivs = 0;

    if (sp->sct_effic < 20) {
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
	    } else {
		/* if oldown gets his cap back make agri */
		if (sp->sct_oldown &&
		    player->cnum == sp->sct_own &&
		    sp->sct_type == SCT_CAPIT &&
		    sp->sct_newtype == SCT_CAPIT)
		    give_back_cap(sp);
		takeover(sp, sp->sct_oldown);
	    }
	    sp->sct_mobil = 0;
	}
    }
    if (!opt_DEFENSE_INFRA)
	sp->sct_defense = sp->sct_effic;
    return 1;
}

int
issector(s_char *arg)
{
    s_char c;

    while (0 != (c = *arg++))
	if (!isdigit(c) && !isspace(c) && (c != '/'))
	    return 1;

    return 0;
}

static void
give_back_cap(struct sctstr *sp)
{
    struct natstr *natp = getnatp(sp->sct_oldown);

    if (xrel(natp, natp->nat_xcap) == xrel(natp, sp->sct_x) &&
	yrel(natp, natp->nat_ycap) == yrel(natp, sp->sct_y)) {
	sp->sct_newtype = SCT_AGRI;
    }
}

void
sct_init(coord x, coord y, s_char *ptr)
{
    struct sctstr *sp = (struct sctstr *)ptr;

    sp->ef_type = EF_SECTOR;
    sp->sct_x = x;
    sp->sct_y = y;
    sp->sct_dist_x = x;
    sp->sct_dist_y = y;
}
